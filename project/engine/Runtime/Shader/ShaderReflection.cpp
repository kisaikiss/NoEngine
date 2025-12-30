#include "ShaderReflection.h"

#include "engine/Utilities/Conversion/ConvertString.h"

#include <d3d12shader.h>

namespace NoEngine {
using namespace Microsoft::WRL;
void ShaderReflection::ReflectShader(const std::vector<uint8_t>& bytecode, ShaderStage stage) {
	stage_ = stage;
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

std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayoutBuilder::BuildFromReflection(const ShaderReflection& refl) {
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
	layout.reserve(refl.inputs_.size());
	
	for (const auto& input : refl.inputs_) {
		
		D3D12_INPUT_ELEMENT_DESC desc = {};
		desc.SemanticName = input.semanticName.c_str();
		desc.SemanticIndex = input.semanticIndex;
		desc.Format = input.format;
		desc.InputSlot = input.inputSlot;
		desc.AlignedByteOffset = input.alignedByteOffset;
		desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;

		layout.push_back(desc);
	}

	return layout;

}

namespace {
std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> rootPramIndexMapMap;
}
void RootSignatureBuilder::BuildFromReflection(
	const std::vector<ShaderReflection>& reflections,
	RootSignature& rootSig,
	const std::string& rootSigName) {
	struct ResourceKey {
		ShaderReflection::ResourceType type;
		uint32_t bindPoint;
		uint32_t space;

		bool operator==(const ResourceKey& other) const noexcept {
			return type == other.type &&
				bindPoint == other.bindPoint &&
				space == other.space;
		}
	};

	struct ResourceKeyHasher {
		size_t operator()(const ResourceKey& k) const noexcept {
			return (static_cast<size_t>(k.type) << 24) ^
				(static_cast<size_t>(k.bindPoint) << 8) ^
				static_cast<size_t>(k.space);
		}
	};

	struct MergedResource {
		ShaderReflection::ResourceType type;
		uint32_t bindPoint;
		uint32_t space;
		uint32_t bindCount;
		std::vector<std::string> names; // このレジスタを使う全シェーダ名
	};

	std::unordered_map<ResourceKey, MergedResource, ResourceKeyHasher> merged;
	bool hasVS = false;

	// まずはステージごとのリフレクション結果をマージ
	for (auto& refl : reflections) {
		if (refl.stage_ == ShaderStage::Vertex) {
			hasVS = true;
		}

		for (auto& r : refl.resources_) {
			ResourceKey key{ r.type, r.bindPoint, r.space };

			auto it = merged.find(key);
			if (it == merged.end()) {
				MergedResource m;
				m.type = r.type;
				m.bindPoint = r.bindPoint;
				m.space = r.space;
				m.bindCount = std::max(r.bindCount, 1u);
				m.names.push_back(r.name);
				merged.emplace(key, std::move(m));
			} else {
				it->second.bindCount = std::max(it->second.bindCount, std::max(r.bindCount, 1u));
				it->second.names.push_back(r.name);
			}
		}
	}

	// Sampler の数だけ先に数える
	uint32_t samplerCount = 0;
	for (auto& [key, r] : merged) {
		if (r.type == ShaderReflection::ResourceType::kSampler) {
			samplerCount++;
		}
	}

	// RootParameter 数 = 全リソース - サンプラー
	rootSig.Reset(static_cast<UINT>(merged.size()) - samplerCount, samplerCount);

	// サンプラー共通設定
	D3D12_SAMPLER_DESC defaultSampler = {};
	defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	defaultSampler.MinLOD = 0.0f;
	defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
	defaultSampler.MipLODBias = 0.0f;
	defaultSampler.MaxAnisotropy = 1;
	defaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// ここからが重要：
	// unordered_map だと順序がバラバラなので、一度 vector に集めてソートする
	std::vector<MergedResource> nonSamplerResources;
	nonSamplerResources.reserve(merged.size() - samplerCount);

	std::vector<MergedResource> samplerResources;
	samplerResources.reserve(samplerCount);

	for (auto& [key, r] : merged) {
		if (r.type == ShaderReflection::ResourceType::kSampler) {
			samplerResources.push_back(r);
		} else {
			nonSamplerResources.push_back(r);
		}
	}

	// RootParameter の並び順を決めるソート
	// 1. CBV
	// 2. SRV（テクスチャなど）
	// 3. UAV（必要になったら追加）
	// その中で space → bindPoint の順に並べる
	std::sort(nonSamplerResources.begin(), nonSamplerResources.end(),
		[](const MergedResource& a, const MergedResource& b) {
			// 型の優先順位
			auto order = [](ShaderReflection::ResourceType t) {
				switch (t) {
				case ShaderReflection::ResourceType::kConstantBuffer:   return 0; // CBV
				case ShaderReflection::ResourceType::kTexture:
				case ShaderReflection::ResourceType::kStructuredBuffer:
				case ShaderReflection::ResourceType::kByteAddressBuffer: return 1; // SRV
				case ShaderReflection::ResourceType::kRWTexture:
				case ShaderReflection::ResourceType::kRWStructuredBuffer:
				case ShaderReflection::ResourceType::kRWByteAddressBuffer: return 2; // UAV
				default: return 3;
				}
				};

			int oa = order(a.type);
			int ob = order(b.type);
			if (oa != ob) return oa < ob;

			if (a.space != b.space) return a.space < b.space;
			return a.bindPoint < b.bindPoint;
		});

	uint32_t rootIndex = 0;
	uint32_t samplerIndex = 0;

	// まず Static Sampler を登録
	for (auto& r : samplerResources) {
		rootSig.InitStaticSampler(
			r.bindPoint,
			defaultSampler,
			D3D12_SHADER_VISIBILITY_ALL
		);

		for (auto& name : r.names) {
			rootPramIndexMapMap[rootSigName][name] = samplerIndex;
		}
		samplerIndex++;
	}

	// 次にソート済みのリソースから RootParameter を構築
	for (auto& r : nonSamplerResources) {
		RootParameter& param = rootSig[rootIndex];

		switch (r.type) {
		case ShaderReflection::ResourceType::kConstantBuffer:
			// register(bN, spaceM)
			param.InitAsConstantBuffer(
				r.bindPoint,                 // bN
				D3D12_SHADER_VISIBILITY_ALL,
				r.space                      // space
			);
			break;

		case ShaderReflection::ResourceType::kTexture:
		case ShaderReflection::ResourceType::kStructuredBuffer:
		case ShaderReflection::ResourceType::kByteAddressBuffer:
			// SRV (tN, spaceM) を必要数だけ
			param.InitAsDescriptorRange(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				r.bindPoint,                 // tN
				r.bindCount,
				D3D12_SHADER_VISIBILITY_ALL,
				r.space
			);
			break;

		case ShaderReflection::ResourceType::kRWTexture:
		case ShaderReflection::ResourceType::kRWStructuredBuffer:
		case ShaderReflection::ResourceType::kRWByteAddressBuffer:
			// 将来 UAV を入れたくなったとき用
			param.InitAsDescriptorRange(
				D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
				r.bindPoint,                 // uN
				r.bindCount,
				D3D12_SHADER_VISIBILITY_ALL,
				r.space
			);
			break;

		default:
			break;
		}

		// このレジスタを使うすべての名前を同じ RootIndex にマップ
		for (auto& name : r.names) {
			rootPramIndexMapMap[rootSigName][name] = rootIndex;
		}

		rootIndex++;
	}

	D3D12_ROOT_SIGNATURE_FLAGS flags =
		hasVS ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		: D3D12_ROOT_SIGNATURE_FLAG_NONE;

	rootSig.Finalize(L"AutoGeneratedRootSignature", flags);
	rootSig.Dump();
}

std::unordered_map<std::string, uint32_t>& RootSignatureBuilder::GetRootIndexMap(std::string rootSigName) {
	return rootPramIndexMapMap[rootSigName];
}

}
