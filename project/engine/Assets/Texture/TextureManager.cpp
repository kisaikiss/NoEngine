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
	ManagedTexture() : isValid_(false), isLoading_(true) {
		cpuDescriptorHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	void WaitForLoad(void) const {
		while ((volatile bool&)isLoading_)
			this_thread::yield();
	}
	void CreateFromMemory(ByteArray memory, eDefaultTexture fallback, bool sRGB);
private:

	bool IsValid(void) const { return isValid_; }
	bool isValid_;
	bool isLoading_;
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
		GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, cpuDescriptorHandle_,
			GetDefaultTexture(fallback), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return;
	}
	if (forceSRGB) metadata.format = DirectX::MakeSRGB(metadata.format);

	hr = DirectX::CreateTexture(GraphicsCore::sGraphicsDevice->GetDevice(), metadata, resource_.GetAddressOf());
	if (FAILED(hr)) {
		GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, cpuDescriptorHandle_,
			GetDefaultTexture(fallback), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return;
	}

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	hr = DirectX::PrepareUpload(GraphicsCore::sGraphicsDevice->GetDevice(),
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

	GraphicsCore::sGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), &srvDesc, cpuDescriptorHandle_);

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
// キャッシュは所有せず、最後のTextureRefが解放された時点でテクスチャを破棄する。
unordered_map<wstring, weak_ptr<ManagedTexture>> sTextureCache;
mutex sMutex;
}
void TextureManager::Initialize(const std::wstring& TextureRoot) {
	sRootPath = TextureRoot;
}
void TextureManager::Shutdown() {
	lock_guard<mutex> Guard(sMutex);
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
TextureRef TextureManager::LoadCovertTexture(const std::wstring& filePath, eDefaultTexture fallback, bool forceSRGB) {
	std::wstring originalFile = filePath;
	CompileTextureOnDemand(originalFile, TextureOptions(true));

	std::wstring ddsFile = RemoveExtension(originalFile) + L".dds";
	return FindOrLoadTexture(ddsFile, fallback, forceSRGB);
}

TextureRef TextureManager::LoadCovertTexture(const std::string& filePath, eDefaultTexture fallback, bool forceSRGB) {
	return LoadCovertTexture(ConvertString(filePath), fallback, forceSRGB);
}

std::shared_ptr<ManagedTexture> TextureManager::FindOrLoadTexture(const std::wstring& fileName, eDefaultTexture fallback, bool forceSRGB) {
	std::shared_ptr<ManagedTexture> tex;

	{
		std::lock_guard<std::mutex> Guard(sMutex);

		std::wstring key = fileName;
		if (forceSRGB)
			key += L"_sRGB";

		auto iter = sTextureCache.find(key);
		if (iter != sTextureCache.end()) {
			tex = iter->second.lock();
			if (tex) {
				tex->WaitForLoad();
				return tex;
			}
		}

		tex = std::make_shared<ManagedTexture>();
		sTextureCache[key] = tex;
	}

	ByteArray ba = ReadFileSync(sRootPath + fileName);
	tex->CreateFromMemory(ba, fallback, forceSRGB);

	// これは初めての要求なので、呼び出し側がファイルを読み取る必要があることを示します。
	return tex;
}

bool TextureManager::UpdatePixels(const std::wstring& key, uint32_t width, uint32_t height,
	DXGI_FORMAT format, const void* pixelData, size_t rowPitchBytes) {
	std::shared_ptr<ManagedTexture> tex;
	{
		std::lock_guard<std::mutex> Guard(sMutex);
		auto iter = sTextureCache.find(key);
		if (iter == sTextureCache.end()) {
			return false;
		}
		tex = iter->second.lock();
		if (!tex) {
			return false;
		}
	}

	tex->WaitForLoad();
	// ToDo: 本来はGPU側の参照完了をフェンスで待ってから差し替えるべき。
	// 現状はフォントへの新規文字追加(低頻度)専用の簡易実装として許容している。
	tex->Create2D(rowPitchBytes, width, height, format, pixelData);
	return true;
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
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

TextureRef TextureManager::CreateFromPixels(const std::wstring& key, uint32_t width, uint32_t height,
	DXGI_FORMAT format, const void* pixelData, size_t rowPitchBytes) {
	std::shared_ptr<ManagedTexture> tex;
	{
		std::lock_guard<std::mutex> Guard(sMutex);

		auto iter = sTextureCache.find(key);
		if (iter != sTextureCache.end()) {
			tex = iter->second.lock();
			if (tex) {
				tex->WaitForLoad();
				return tex;
			}
		}

		tex = std::make_shared<ManagedTexture>();
		sTextureCache[key] = tex;
	}

	// ManagedTextureはTextureを継承しているので、Create2D()をそのまま使ってGPUへアップロードする
	tex->Create2D(rowPitchBytes, width, height, format, pixelData);
	tex->isValid_ = true;
	tex->isLoading_ = false;

	return tex;
}

#pragma endregion

#pragma region TextureRef
TextureRef::TextureRef(const TextureRef& ref) = default;
TextureRef::TextureRef(TextureRef&& ref) noexcept = default;
TextureRef::TextureRef(std::shared_ptr<ManagedTexture> tex) : ref_(std::move(tex)) {}
TextureRef::~TextureRef() = default;
void TextureRef::operator=(std::nullptr_t) {
	ref_.reset();
}
TextureRef& TextureRef::operator=(const TextureRef& rhs) = default;
TextureRef& TextureRef::operator=(TextureRef&& rhs) noexcept = default;
bool TextureRef::IsValid() const {
	return ref_ && ref_->IsValid();
}
uint32_t TextureRef::GetWidth() {
	return ref_ ? ref_->GetWidth() : 0;
}
uint32_t TextureRef::GetHeight() {
	return ref_ ? ref_->GetHeight() : 0;
}
const Texture* TextureRef::Get(void) const {
	return ref_.get();
}
const Texture* TextureRef::operator->(void) const {
	assert(ref_ != nullptr);
	return ref_.get();
}
D3D12_CPU_DESCRIPTOR_HANDLE TextureRef::GetSRV() const {
	if (ref_ != nullptr)
		return ref_->GetSRV();
	else
		return GetDefaultTexture(kMagenta2D);
}
#pragma endregion
}