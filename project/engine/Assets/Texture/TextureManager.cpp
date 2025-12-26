#include "TextureManager.h"

#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Utilities/FileUtilities.h"
#include "engine/Runtime/Graphics/GraphicsCommon.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
using namespace Utilities;
using namespace Graphics;

#pragma region ManagedTexture
class ManagedTexture : public Texture {
	friend class TextureRef;

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
	(void)forceSRGB;
	if (ba->size() == 0) {
		cpuDescriptorHandle_ = GetDefaultTexture(fallback);
	} else {
		// We probably have a texture to load, so let's allocate a new descriptor
		cpuDescriptorHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	/*	if (SUCCEEDED(CreateDDSTextureFromMemory(GraphicsCore::gGraphicsDevice->GetDevice(), (const uint8_t*)ba->data(), ba->size(),
			0, forceSRGB, m_pResource.GetAddressOf(), cpuDescriptorHandle_))) {
			m_UsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
			m_IsValid = true;
			D3D12_RESOURCE_DESC desc = GetResource()->GetDesc();
			m_Width = (uint32_t)desc.Width;
			m_Height = desc.Height;
			m_Depth = desc.DepthOrArraySize;
		} else {
			GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, cpuDescriptorHandle_, GetDefaultTexture(fallback),
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}*/
	}

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