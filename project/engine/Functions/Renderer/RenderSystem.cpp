#include "RenderSystem.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
DescriptorHeap gTextureHeap;

namespace {

std::vector<std::unique_ptr<RootSignature>> sRootSignatures;
std::unordered_map<std::wstring, uint32_t> sRootSignatureIndexMap;

Microsoft::WRL::ComPtr<ID3D12StateObject> sRtShadowStateObject;
D3D12_DISPATCH_RAYS_DESC sShadowDispatchRaysDesc;

Microsoft::WRL::ComPtr<ID3D12Resource> sShadowShaderTable;

Microsoft::WRL::ComPtr<ID3D12StateObject> sRtTestStateObject;
D3D12_DISPATCH_RAYS_DESC sTestDispatchRaysDesc;

Microsoft::WRL::ComPtr<ID3D12Resource> sTestShaderTable;
}

void Initialize() {
	gTextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
	// ToDo : 現在はシェーダーコンパイル、PSO生成をここで行っていますが、アプリケーション側で動的に行えるようにするべきです。
	ShaderModule::Initialize();



	// RayTest
	{
		ShaderModule raytracingLib(
			ShaderStage::RaytraceLib,
			L"resources/engine/Shaders/Raytracing/RaytracingTestLib.hlsl",
			L"lib_6_3",
			true);

		InitRaytracingGlobalRootSignature();

		std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		subObjects.reserve(5);

		// 1. DXIL ライブラリ
		D3D12_EXPORT_DESC exports[3] = {};

		// RayGen
		exports[0].Name = L"RayGen";
		exports[0].ExportToRename = nullptr;
		exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

		// Miss
		exports[1].Name = L"MissShader";
		exports[1].ExportToRename = nullptr;
		exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

		// ClosestHit
		exports[2].Name = L"ClosestHitShader";
		exports[2].ExportToRename = nullptr;
		exports[2].Flags = D3D12_EXPORT_FLAG_NONE;


		D3D12_DXIL_LIBRARY_DESC dxilLib = {};
		dxilLib.DXILLibrary = raytracingLib.GetBytecode();
		dxilLib.NumExports = _countof(exports);
		dxilLib.pExports = exports;

		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &dxilLib;
		subObjects.push_back(libSubobject);

		// 2. HitGroup
		static const wchar_t* kHitGroupExport = L"TestHitGroup";
		static const wchar_t* kClosestHitExport = L"ClosestHitShader";

		D3D12_HIT_GROUP_DESC hitGroup = {};
		hitGroup.HitGroupExport = kHitGroupExport;
		hitGroup.ClosestHitShaderImport = kClosestHitExport;
		hitGroup.AnyHitShaderImport = nullptr;
		hitGroup.IntersectionShaderImport = nullptr;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

		D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
		hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroupSubobject.pDesc = &hitGroup;
		subObjects.push_back(hitGroupSubobject);

		// 3. ShaderConfig（Association なし＝全シェーダーに適用）
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = 16;                 // float 4つ
		shaderConfig.MaxAttributeSizeInBytes = sizeof(float) * 2; // barycentrics

		D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
		shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubobject.pDesc = &shaderConfig;
		subObjects.push_back(shaderConfigSubobject);

		// 4. Global RootSignature
		// 構造体を作成し、そこにRootSignatureのポインタをセットする

		std::unique_ptr<RootSignature> rtGlobalRSptr = std::make_unique<RootSignature>();
		auto& rtGlobalRS = *rtGlobalRSptr.get();

		rtGlobalRS.Reset(3, 0);

		// TLAS
		rtGlobalRS[0].InitAsBufferSRV(0);

		// UAV
		rtGlobalRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);

		// CBV b0
		rtGlobalRS[2].InitAsConstantBuffer(0);


		rtGlobalRS.Finalize(L"RT Test Global RootSignature");
		sRootSignatures.push_back(std::move(rtGlobalRSptr));
		sRootSignatureIndexMap[L"RT Test Global RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

		D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc = {};
		globalRSDesc.pGlobalRootSignature = sRootSignatures[sRootSignatureIndexMap[L"RT Test Global RootSignature"]]->GetSignature();

		D3D12_STATE_SUBOBJECT globalRSSubobject = {};
		globalRSSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		// 構造体のアドレスを渡す
		globalRSSubobject.pDesc = &globalRSDesc;
		subObjects.push_back(globalRSSubobject);

		// 5. PipelineConfig
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 1;

		D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
		pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigSubobject.pDesc = &pipelineConfig;
		subObjects.push_back(pipelineConfigSubobject);

		// 6. StateObject 生成
		D3D12_STATE_OBJECT_DESC desc = {};
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		desc.NumSubobjects = static_cast<UINT>(subObjects.size());
		desc.pSubobjects = subObjects.data();

		HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateStateObject(&desc, IID_PPV_ARGS(&sRtTestStateObject));

		if (FAILED(hr)) {
			assert(false);
		}
		

		using namespace Microsoft::WRL;
		// 1. シェーダー識別子を取る
		ComPtr<ID3D12StateObjectProperties> props;
		sRtTestStateObject.As(&props);

		void* raygenId = props->GetShaderIdentifier(L"RayGen");
		void* missId = props->GetShaderIdentifier(L"MissShader");
		void* hitId = props->GetShaderIdentifier(L"TestHitGroup");

		// 2.ShaderTableバッファ作成
		const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
		const UINT recordStride = (shaderIdSize + (D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1))
			& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 32

		const UINT tableAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64

		// 各テーブルが64バイトアライメントの境界から始まるようにセクションサイズを計算
		const UINT rayGenSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
		const UINT missSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
		const UINT hitGroupSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1); // 64

		// オフセットの計算
		const UINT rayGenOffset = 0;
		const UINT missOffset = rayGenOffset + rayGenSectionSize;     // 64
		const UINT hitGroupOffset = missOffset + missSectionSize;         // 128

		const UINT totalTableSizeUnaligned = hitGroupOffset + hitGroupSectionSize; // 192

		// バッファ先頭のアライメント調整用の余白(+64)を含めたサイズ
		const UINT tableSize = totalTableSizeUnaligned + tableAlignment;

		// Create upload buffer
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
		GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&sTestShaderTable));

		// Map and write identifiers at offsets relative to an aligned base
		uint8_t* mapped = nullptr;
		sTestShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
		UINT64 gpuBase = sTestShaderTable->GetGPUVirtualAddress();

		// compute aligned GPU base (>= gpuBase) aligned to 64
		UINT64 alignedGpuBase = (gpuBase + (tableAlignment - 1)) & ~(UINT64)(tableAlignment - 1);

		// compute CPU offset corresponding to alignedGpuBase
		SIZE_T cpuOffset = static_cast<SIZE_T>(alignedGpuBase - gpuBase);

		// 計算したアライメント済みのオフセットを使って書き込む
		memcpy(mapped + cpuOffset + rayGenOffset, raygenId, shaderIdSize);
		memcpy(mapped + cpuOffset + missOffset, missId, shaderIdSize);
		memcpy(mapped + cpuOffset + hitGroupOffset, hitId, shaderIdSize);

		sTestShaderTable->Unmap(0, nullptr);

		// Fill dispatch desc using alignedGpuBase
		auto& dispatchRayDesc = sTestDispatchRaysDesc;

		// 各StartAddressに計算したオフセットを足す
		dispatchRayDesc.RayGenerationShaderRecord.StartAddress = alignedGpuBase + rayGenOffset;
		dispatchRayDesc.RayGenerationShaderRecord.SizeInBytes = recordStride;

		dispatchRayDesc.MissShaderTable.StartAddress = alignedGpuBase + missOffset;
		dispatchRayDesc.MissShaderTable.SizeInBytes = recordStride;
		dispatchRayDesc.MissShaderTable.StrideInBytes = recordStride;

		dispatchRayDesc.HitGroupTable.StartAddress = alignedGpuBase + hitGroupOffset;
		dispatchRayDesc.HitGroupTable.SizeInBytes = recordStride;
		dispatchRayDesc.HitGroupTable.StrideInBytes = recordStride;

		dispatchRayDesc.Width = 1280;
		dispatchRayDesc.Height = 720;
		dispatchRayDesc.Depth = 1;
	}

	// RayShadow
	{
		
		ShaderModule raytracingLib(
			ShaderStage::RaytraceLib, 
			L"resources/engine/Shaders/Raytracing/RayTracingShadowLib.hlsl",
			L"lib_6_3",
			true);
	
		InitRaytracingGlobalRootSignature();

		std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		subObjects.reserve(5);

		// 1. DXIL ライブラリ
		D3D12_EXPORT_DESC exports[3] = {};

		// RayGen
		exports[0].Name = L"RayGen_Shadow";
		exports[0].ExportToRename = nullptr;
		exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

		// Miss
		exports[1].Name = L"Miss_Shadow";
		exports[1].ExportToRename = nullptr;
		exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

		// ClosestHit
		exports[2].Name = L"ClosestHit_Shadow";
		exports[2].ExportToRename = nullptr;
		exports[2].Flags = D3D12_EXPORT_FLAG_NONE;


		D3D12_DXIL_LIBRARY_DESC dxilLib = {};
		dxilLib.DXILLibrary = raytracingLib.GetBytecode();
		dxilLib.NumExports = _countof(exports);
		dxilLib.pExports = exports;

		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &dxilLib;
		subObjects.push_back(libSubobject);

		// 2. HitGroup
		static const wchar_t* kHitGroupExport = L"ShadowHitGroup";
		static const wchar_t* kClosestHitExport = L"ClosestHit_Shadow";

		D3D12_HIT_GROUP_DESC hitGroup = {};
		hitGroup.HitGroupExport = kHitGroupExport;
		hitGroup.ClosestHitShaderImport = kClosestHitExport;
		hitGroup.AnyHitShaderImport = nullptr;
		hitGroup.IntersectionShaderImport = nullptr;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

		D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
		hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroupSubobject.pDesc = &hitGroup;
		subObjects.push_back(hitGroupSubobject);

		// 3. ShaderConfig（Association なし＝全シェーダーに適用）
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = 4;                 // bool 1つ
		shaderConfig.MaxAttributeSizeInBytes = sizeof(float) * 2; // barycentrics

		D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
		shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubobject.pDesc = &shaderConfig;
		subObjects.push_back(shaderConfigSubobject);

		// 4. Global RootSignature
		// 構造体を作成し、そこにRootSignatureのポインタをセットする
		D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc = {};
		globalRSDesc.pGlobalRootSignature = sRootSignatures[sRootSignatureIndexMap[L"RT Global RootSignature"]]->GetSignature();

		D3D12_STATE_SUBOBJECT globalRSSubobject = {};
		globalRSSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		// 構造体のアドレスを渡す
		globalRSSubobject.pDesc = &globalRSDesc;
		subObjects.push_back(globalRSSubobject);

		// 5. PipelineConfig
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 1;

		D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
		pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigSubobject.pDesc = &pipelineConfig;
		subObjects.push_back(pipelineConfigSubobject);

		// 6. StateObject 生成
		D3D12_STATE_OBJECT_DESC desc = {};
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		desc.NumSubobjects = static_cast<UINT>(subObjects.size());
		desc.pSubobjects = subObjects.data();

		HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateStateObject(&desc, IID_PPV_ARGS(&sRtShadowStateObject));

		if (FAILED(hr)) {
			assert(false);
		}
		CreateShadowShaderTable();
	}
	
}

void Shutdown() {
	sShadowShaderTable.Reset();
	sRtShadowStateObject.Reset();

	sTestShaderTable.Reset();
	sRtTestStateObject.Reset();

	gTextureHeap.Destroy();
	ModelSaver::Get().ClearAll();
	PSO::DestroyAll();
	RootSignature::DestroyAll();
	ShaderModule::Shutdown();
}

RootSignature& GetRootSignature(uint32_t rootSigId) {
	return *sRootSignatures[rootSigId];
}

uint32_t GetRootSignatureID(std::wstring rootSigName) {
	return sRootSignatureIndexMap[rootSigName];
}

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetShadowRtStateObject() {
	return sRtShadowStateObject;
}

D3D12_DISPATCH_RAYS_DESC& GetShadowDispatchRaysDesc() {
	return sShadowDispatchRaysDesc;
}

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetRtStateObject() {
	return sRtTestStateObject;
}

D3D12_DISPATCH_RAYS_DESC& GetDispatchRaysDesc() {
	return sTestDispatchRaysDesc;
}

void CreateShadowShaderTable() {
	using namespace Microsoft::WRL;
	// 1. シェーダー識別子を取る
	ComPtr<ID3D12StateObjectProperties> props;
	sRtShadowStateObject.As(&props);

	void* raygenId = props->GetShaderIdentifier(L"RayGen_Shadow");
	void* missId = props->GetShaderIdentifier(L"Miss_Shadow");
	void* hitId = props->GetShaderIdentifier(L"ShadowHitGroup");

	// 2.ShaderTableバッファ作成
	const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
	const UINT recordStride = (shaderIdSize + (D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1))
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 32

	const UINT tableAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64

	// 各テーブルが64バイトアライメントの境界から始まるようにセクションサイズを計算
	const UINT rayGenSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
	const UINT missSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
	const UINT hitGroupSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1); // 64

	// オフセットの計算
	const UINT rayGenOffset = 0;
	const UINT missOffset = rayGenOffset + rayGenSectionSize;     // 64
	const UINT hitGroupOffset = missOffset + missSectionSize;         // 128

	const UINT totalTableSizeUnaligned = hitGroupOffset + hitGroupSectionSize; // 192

	// バッファ先頭のアライメント調整用の余白(+64)を含めたサイズ
	const UINT tableSize = totalTableSizeUnaligned + tableAlignment;

	// Create upload buffer
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&sShadowShaderTable));

	// Map and write identifiers at offsets relative to an aligned base
	uint8_t* mapped = nullptr;
	sShadowShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	UINT64 gpuBase = sShadowShaderTable->GetGPUVirtualAddress();

	// compute aligned GPU base (>= gpuBase) aligned to 64
	UINT64 alignedGpuBase = (gpuBase + (tableAlignment - 1)) & ~(UINT64)(tableAlignment - 1);

	// compute CPU offset corresponding to alignedGpuBase
	SIZE_T cpuOffset = static_cast<SIZE_T>(alignedGpuBase - gpuBase);

	// 計算したアライメント済みのオフセットを使って書き込む
	memcpy(mapped + cpuOffset + rayGenOffset, raygenId, shaderIdSize);
	memcpy(mapped + cpuOffset + missOffset, missId, shaderIdSize);
	memcpy(mapped + cpuOffset + hitGroupOffset, hitId, shaderIdSize);

	sShadowShaderTable->Unmap(0, nullptr);

	// Fill dispatch desc using alignedGpuBase
	auto& desc = sShadowDispatchRaysDesc;

	// 各StartAddressに計算したオフセットを足す
	desc.RayGenerationShaderRecord.StartAddress = alignedGpuBase + rayGenOffset;
	desc.RayGenerationShaderRecord.SizeInBytes = recordStride;

	desc.MissShaderTable.StartAddress = alignedGpuBase + missOffset;
	desc.MissShaderTable.SizeInBytes = recordStride;
	desc.MissShaderTable.StrideInBytes = recordStride;

	desc.HitGroupTable.StartAddress = alignedGpuBase + hitGroupOffset;
	desc.HitGroupTable.SizeInBytes = recordStride;
	desc.HitGroupTable.StrideInBytes = recordStride;

	desc.Width = 1280;
	desc.Height = 720;
	desc.Depth = 1;
}

void InitRaytracingGlobalRootSignature() {
	std::unique_ptr<RootSignature> rtGlobalRSptr = std::make_unique<RootSignature>();
	auto& rtGlobalRS = *rtGlobalRSptr.get();

	rtGlobalRS.Reset(9, 0); // 7 → 9

	rtGlobalRS[0].InitAsBufferSRV(0);                                              // TLAS
	rtGlobalRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);     // worldPos
	rtGlobalRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);     // Directional Lights
	rtGlobalRS[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);     // Shadow Mask (Array)
	rtGlobalRS[4].InitAsConstantBuffer(0);                                         // CameraCB
	rtGlobalRS[5].InitAsConstantBuffer(1);                                         // LightNums
	rtGlobalRS[6].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1);     // Normal
	rtGlobalRS[7].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1);     // Point Lights
	rtGlobalRS[8].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1);     // Spot Lights

	rtGlobalRS.Finalize(L"RT Global RootSignature");
	sRootSignatures.push_back(std::move(rtGlobalRSptr));
	sRootSignatureIndexMap[L"RT Global RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
}

void InitRaytracingLocalRootSignature() {
	std::unique_ptr<RootSignature> rtLocalRSptr = std::make_unique<RootSignature>();
	auto& rtLocalRS = *rtLocalRSptr.get();

	rtLocalRS.Reset(1, 0);

	// materialIndex, instanceID の 2 つを渡す
	rtLocalRS[0].InitAsConstants(
		0,      // b0
		2,      // 2 DWORD
		D3D12_SHADER_VISIBILITY_ALL,
		1       // space1 (local root)
	);

	rtLocalRS.Finalize(
		L"RT Local RootSignature",
		D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE
	);

	sRootSignatures.push_back(std::move(rtLocalRSptr));
	sRootSignatureIndexMap[L"RT Local RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
}
}
}