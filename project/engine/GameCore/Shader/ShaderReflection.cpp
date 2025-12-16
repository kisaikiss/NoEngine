#include "ShaderReflection.h"

#include "engine/Utilities/Conversion/ConvertString.h"

#include <d3d12shader.h>

namespace NoEngine {
using namespace Microsoft::WRL;
void ShaderReflection::ReflectShader(const std::vector<uint8_t>& bytecode) {
	// DXILコンテナReflection
	ComPtr<IDxcContainerReflection> container;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&container));

	// bytecodeをIDxcBlobへ変換します。
	ComPtr<IDxcLibrary> library;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	ComPtr<IDxcBlobEncoding> blobEnc;
	library->CreateBlobWithEncodingOnHeapCopy(
		bytecode.data(),
		static_cast<UINT32>(bytecode.size()),
		CP_UTF8,   // バイナリなので実際は何でも良い
		&blobEnc
	);

	ComPtr<IDxcBlob> blob;
	blobEnc.As(&blob);

	// DXILバイトコードをコンテナとしてロードします。
	container->Load(blob.Get());

	// シェーダーパートのインデックスを取得します。
	UINT shaderIdx = 0;
	container->FindFirstPartKind(DXC_PART_DXIL, &shaderIdx);

	// シェーダーリフレクションを取得します。
	ComPtr<ID3D12ShaderReflection> refl;
	container->GetPartReflection(shaderIdx, IID_PPV_ARGS(&refl));

	//  シェーダー全体の情報を取得します。
	D3D12_SHADER_DESC desc = {};
	refl->GetDesc(&desc);

	// インプットレイアウト
	for (UINT i = 0; i < desc.InputParameters; i++) {
		D3D12_SIGNATURE_PARAMETER_DESC ip = {};
		refl->GetInputParameterDesc(i, &ip);

		ShaderReflection::InputParameter param;
		param.semanticName = ip.SemanticName;
		param.semanticIndex = ip.SemanticIndex;
		param.format = GuessFormatFromMask(ip.Mask, ip.ComponentType);
		param.inputSlot = 0;
		param.alignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputs_.push_back(param);
	}

	// リソースバインディング
	for (UINT i = 0; i < desc.BoundResources; i++) {
		D3D12_SHADER_INPUT_BIND_DESC rb = {};
		refl->GetResourceBindingDesc(i, &rb);

		ShaderReflection::ResourceBinding res;
		res.name = rb.Name;
		res.bindPoint = rb.BindPoint;
		res.bindCount = rb.BindCount;
		res.space = rb.Space;
		res.type = ConvertResourceType(rb.Type);

		resources_.push_back(res);

		// RootSignature 自動生成用にも保存
		ShaderReflection::RootParameterInfo rp;
		rp.type = res.type;
		rp.bindPoint = res.bindPoint;
		rp.space = res.space;
		rootParams_.push_back(rp);
	}

	// Compute Shader のスレッドグループサイズ
	UINT x = 0, y = 0, z = 0;
	refl->GetThreadGroupSize(&x, &y, &z);
	threadGroupX_ = x;
	threadGroupY_ = y;
	threadGroupZ_ = z;

	// デバッグ用のダンプを作成
	BuildDebugDump();
}
DXGI_FORMAT ShaderReflection::GuessFormatFromMask(UINT mask, D3D_REGISTER_COMPONENT_TYPE type) {

	if (type == D3D_REGISTER_COMPONENT_FLOAT32) {
		int count = 0;
		if (mask & 1) count++;
		if (mask & 2) count++;
		if (mask & 4) count++;
		if (mask & 8) count++;

		switch (count) {
		case 1: return DXGI_FORMAT_R32_FLOAT;
		case 2: return DXGI_FORMAT_R32G32_FLOAT;
		case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}
	return DXGI_FORMAT_UNKNOWN;
}
ShaderReflection::ResourceType ShaderReflection::ConvertResourceType(D3D_SHADER_INPUT_TYPE t) {
	switch (t) {
	case D3D_SIT_CBUFFER: return ShaderReflection::ResourceType::kConstantBuffer;
	case D3D_SIT_TEXTURE: return ShaderReflection::ResourceType::kTexture;
	case D3D_SIT_SAMPLER: return ShaderReflection::ResourceType::kSampler;
	case D3D_SIT_UAV_RWTYPED: return ShaderReflection::ResourceType::kRWTexture;
	case D3D_SIT_STRUCTURED: return ShaderReflection::ResourceType::kStructuredBuffer;
	case D3D_SIT_UAV_RWSTRUCTURED: return ShaderReflection::ResourceType::kRWStructuredBuffer;
	case D3D_SIT_BYTEADDRESS: return ShaderReflection::ResourceType::kByteAddressBuffer;
	case D3D_SIT_UAV_RWBYTEADDRESS: return ShaderReflection::ResourceType::kRWByteAddressBuffer;
	}
	return ShaderReflection::ResourceType::kUnknown;

}

void ShaderReflection::BuildDebugDump() {
	std::ostringstream oss;

	// ============================
	// 1. 基本情報
	// ============================
	oss << "=== Shader Reflection Dump ===\n";
	oss << "EntryPoint: " << entryPoint_ << "\n";
	oss << "TargetProfile: " << targetProfile_ << "\n";
	oss << "\n";

	// ============================
	// 2. 入力レイアウト（VS）
	// ============================
	if (!inputs_.empty()) {
		oss << "[Input Parameters]\n";
		for (const auto& ip : inputs_) {
			oss << "  - " << ip.semanticName;
			if (ip.semanticIndex > 0)
				oss << ip.semanticIndex;
			oss << " : Format=" << FormatToString(ip.format)
				<< ", Slot=" << ip.inputSlot
				<< ", Offset=" << ip.alignedByteOffset
				<< "\n";
		}
		oss << "\n";
	}

	// ============================
	// 3. リソースバインディング
	// ============================
	if (!resources_.empty()) {
		oss << "[Resource Bindings]\n";
		for (const auto& r : resources_) {
			oss << "  - " << r.name
				<< " : Type=" << ResourceTypeToString(r.type)
				<< ", Register=" << r.bindPoint
				<< ", Space=" << r.space
				<< ", Count=" << r.bindCount
				<< "\n";
		}
		oss << "\n";
	}

	// ============================
	// 4. Compute Shader のスレッドグループ
	// ============================
	if (threadGroupX_ > 0) {
		oss << "[Compute Thread Group]\n";
		oss << "  numthreads("
			<< threadGroupX_ << ", "
			<< threadGroupY_ << ", "
			<< threadGroupZ_ << ")\n\n";
	}

	// ============================
	// 5. RootSignature 推奨構造
	// ============================
	if (!rootParams_.empty()) {
		oss << "[RootSignature Parameters]\n";
		for (const auto& rp : rootParams_) {
			oss << "  - " << ResourceTypeToString(rp.type)
				<< " : Register=" << rp.bindPoint
				<< ", Space=" << rp.space
				<< "\n";
		}
		oss << "\n";
	}

	oss << "==============================\n";

	debugDump_ = oss.str();
}

std::string ShaderReflection::ResourceTypeToString(ShaderReflection::ResourceType t) {
	switch (t) {
	case ShaderReflection::ResourceType::kConstantBuffer: return "CBV";
	case ShaderReflection::ResourceType::kTexture: return "SRV";
	case ShaderReflection::ResourceType::kSampler: return "Sampler";
	case ShaderReflection::ResourceType::kRWTexture: return "UAV";
	case ShaderReflection::ResourceType::kStructuredBuffer: return "StructuredBuffer";
	case ShaderReflection::ResourceType::kRWStructuredBuffer: return "RWStructuredBuffer";
	case ShaderReflection::ResourceType::kByteAddressBuffer: return "ByteAddressBuffer";
	case ShaderReflection::ResourceType::kRWByteAddressBuffer: return "RWByteAddressBuffer";
	default: return "Unknown";
	}

}
}
