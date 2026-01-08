#include "TextureManager.h"

#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Utilities/FileUtilities.h"
#include "engine/Runtime/GraphicsCore.h"
#include "TexUtil.h"

namespace NoEngine {
using namespace Utilities;
using namespace Graphics;

#pragma region ManagedTexture
class ManagedTexture : public Texture {
	friend class TextureRef;
	friend class TextureManager;
public:
	ManagedTexture(const wstring& FileName) : mapKey_(FileName), isValid_(false), isLoading_(true), referenceCount_(0) {
		cpuDescriptorHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	void WaitForLoad(void) const {
		while ((volatile bool&)isLoading_)
			this_thread::yield();
	}
	void CreateFromMemory(ByteArray memory, eDefaultTexture fallback, bool sRGB);
private:

	bool IsValid(void) const { return isValid_; }
	void Unload() {
		TextureManager::DestroyTexture(mapKey_);
	}

	std::wstring mapKey_;		// 後でマップから削除するために使用します。
	bool isValid_;
	bool isLoading_;
	size_t referenceCount_;
};

void ManagedTexture::CreateFromMemory(ByteArray ba, eDefaultTexture fallback, bool forceSRGB) {
	if (ba->empty()) {
		cpuDescriptorHandle_ = GetDefaultTexture(fallback);
		isLoading_ = false;
		return;
	}

	cpuDescriptorHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratch = {};

	HRESULT hr = DirectX::LoadFromDDSMemory(ba->data(), ba->size(), DirectX::DDS_FLAGS_NONE, &metadata, scratch);
	if (FAILED(hr)) {
		GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, cpuDescriptorHandle_,
			GetDefaultTexture(fallback), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return;
	}
	if (forceSRGB) metadata.format = DirectX::MakeSRGB(metadata.format);

	hr = DirectX::CreateTexture(GraphicsCore::gGraphicsDevice->GetDevice(), metadata, resource_.GetAddressOf());
	if (FAILED(hr)) {
		GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, cpuDescriptorHandle_,
			GetDefaultTexture(fallback), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return;
	}

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	hr = DirectX::PrepareUpload(GraphicsCore::gGraphicsDevice->GetDevice(),
		scratch.GetImages(), scratch.GetImageCount(), metadata, subresources);
	if (FAILED(hr)) return;

	GpuResource gpuTex(resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
	CommandContext::InitializeTexture(gpuTex, (UINT)subresources.size(), subresources.data());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;

	switch (metadata.dimension) {
	case DirectX::TEX_DIMENSION_TEXTURE1D:
		if (metadata.arraySize > 1) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			srvDesc.Texture1DArray.MipLevels = (UINT)metadata.mipLevels;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MipLevels = (UINT)metadata.mipLevels;
		}
		break;

	case DirectX::TEX_DIMENSION_TEXTURE2D:
		if (metadata.IsCubemap()) {
			if (metadata.arraySize > 6) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				srvDesc.TextureCubeArray.MipLevels = (UINT)metadata.mipLevels;
			} else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MipLevels = (UINT)metadata.mipLevels;
			}
		} else {
			if (metadata.arraySize > 1) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = (UINT)metadata.mipLevels;
			} else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;
			}
		}
		break;

	case DirectX::TEX_DIMENSION_TEXTURE3D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = (UINT)metadata.mipLevels;
		break;
	}

	GraphicsCore::gGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), &srvDesc, cpuDescriptorHandle_);

	usageState_ = D3D12_RESOURCE_STATE_GENERIC_READ;
	isValid_ = true;
	width_ = (uint32_t)metadata.width;
	height_ = (uint32_t)metadata.height;
	depth_ = (uint32_t)metadata.depth;
	isLoading_ = false;
}

#pragma endregion

#pragma region TextureManager
namespace {
wstring sRootPath = L"";
unordered_map<wstring, unique_ptr<ManagedTexture>> sTextureCache;
mutex sMutex;
}
void TextureManager::Initialize(const std::wstring& TextureRoot) {
	sRootPath = TextureRoot;
}
void TextureManager::Shutdown() {
	sTextureCache.clear();
}
void TextureManager::DestroyTexture(std::wstring mapKey) {
	lock_guard<mutex> Guard(sMutex);

	auto iter = sTextureCache.find(mapKey);
	if (iter != sTextureCache.end())
		sTextureCache.erase(iter);
}
TextureRef TextureManager::LoadTextureFile(const std::wstring& filePath, eDefaultTexture fallback, bool sRGB) {
	return FindOrLoadTexture(filePath, fallback, sRGB);
}
TextureRef TextureManager::LoadTextureFile(const std::string& filePath, eDefaultTexture fallback, bool sRGB) {
	return LoadTextureFile(ConvertString(filePath), fallback, sRGB);
}
TextureRef TextureManager::LoadCovertTexture(const std::wstring& filePath, eDefaultTexture fallback, bool forceSRGB)
{
	std::wstring originalFile = filePath;
	CompileTextureOnDemand(originalFile, TextureOptions(true));

	std::wstring ddsFile = RemoveExtension(originalFile) + L".dds";
	return FindOrLoadTexture(ddsFile, fallback, forceSRGB);
}

TextureRef TextureManager::LoadCovertTexture(const std::string& filePath, eDefaultTexture fallback, bool forceSRGB)
{
	return LoadCovertTexture(ConvertString(filePath), fallback, forceSRGB);
}

ManagedTexture* TextureManager::FindOrLoadTexture(const std::wstring& fileName, eDefaultTexture fallback, bool forceSRGB) {
	ManagedTexture* tex = nullptr;

	{
		std::lock_guard<std::mutex> Guard(sMutex);

		std::wstring key = fileName;
		if (forceSRGB)
			key += L"_sRGB";

		// 既存の管理テクスチャを検索する
		auto iter = sTextureCache.find(key);
		if (iter != sTextureCache.end()) {
			// テクスチャがすでに作成されている場合は、テクスチャにポイントを返す前に、読み込みが完了していることを確認
			tex = iter->second.get();
			tex->WaitForLoad();
			return tex;
		} else {
			// 見つからない場合は、新しい管理テクスチャを作成し、読み込みを開始します
			tex = new ManagedTexture(key);
			sTextureCache[key].reset(tex);
		}
	}

	ByteArray ba = ReadFileSync(sRootPath + fileName);
	tex->CreateFromMemory(ba, fallback, forceSRGB);

	// これは初めての要求なので、呼び出し側がファイルを読み取る必要があることを示します。
	return tex;
}
DirectX::ScratchImage TextureManager::LordTexture(const std::string& filePath) {
	//テクスチャファイルを読んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミニマップ(元画像より小さなテクスチャ群)の作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミニマップ付きのデータを返す
	return mipImages;
}
#pragma endregion

#pragma region TextureRef
TextureRef::TextureRef(const TextureRef& ref) : ref_(ref.ref_) {
	if (ref_ != nullptr) {
		++ref_->referenceCount_;
	}
}
TextureRef::TextureRef(ManagedTexture* tex) : ref_(tex) {
	if (ref_ != nullptr) {
		++ref_->referenceCount_;
	}
}
TextureRef::~TextureRef() {
	if (ref_ != nullptr && --ref_->referenceCount_ == 0)
		ref_->Unload();
}
void TextureRef::operator= (std::nullptr_t) {
	if (ref_ != nullptr)
		--ref_->referenceCount_;

	ref_ = nullptr;
}
void TextureRef::operator= (TextureRef& rhs) {
	if (ref_ != nullptr)
		--ref_->referenceCount_;

	ref_ = rhs.ref_;

	if (ref_ != nullptr)
		++ref_->referenceCount_;
}
TextureRef& TextureRef::operator=(const TextureRef& rhs) {
	if (this == &rhs)
		return *this;

	if (ref_ != nullptr) {
		--ref_->referenceCount_;
		if (ref_->referenceCount_ == 0)
			ref_->Unload();
	}

	ref_ = rhs.ref_;

	if (ref_ != nullptr)
		++ref_->referenceCount_;

	return *this;
}

TextureRef& TextureRef::operator=(TextureRef&& rhs) noexcept {
	if (this == &rhs)
		return *this;

	if (ref_ != nullptr) {
		--ref_->referenceCount_;
		if (ref_->referenceCount_ == 0)
			ref_->Unload();
	}

	ref_ = rhs.ref_;
	rhs.ref_ = nullptr;

	return *this;
}
bool TextureRef::IsValid() const {
	return ref_ && ref_->IsValid();
}
const Texture* TextureRef::Get(void) const {
	return ref_;
}
const Texture* TextureRef::operator->(void) const {
	assert(ref_ != nullptr);
	return ref_;
}
D3D12_CPU_DESCRIPTOR_HANDLE TextureRef::GetSRV() const {
	if (ref_ != nullptr)
		return ref_->GetSRV();
	else
		return GetDefaultTexture(kMagenta2D);
}
#pragma endregion
}