#include "stdafx.h"
#include "GifTexture.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include <wincodec.h>
#include <wrl/client.h>

namespace {
using Microsoft::WRL::ComPtr;

constexpr UINT kRestoreBackground = 2;
constexpr UINT kRestorePrevious = 3;
constexpr size_t kMaxFrameTextureBytes = 128 * 1024 * 1024;

struct GifFrame {
	std::vector<uint32_t> bgra;
	double durationSeconds;
};

struct GifImage {
	uint32_t width;
	uint32_t height;
	std::vector<GifFrame> frames;
};

UINT ReadMetadata(IWICMetadataQueryReader* reader, const wchar_t* name) {
	PROPVARIANT value{};
	if (FAILED(reader->GetMetadataByName(name, &value))) return 0;

	UINT result = 0;
	switch (value.vt) {
	case VT_UI1: result = value.bVal; break;
	case VT_UI2: result = value.uiVal; break;
	case VT_BOOL: result = value.boolVal != VARIANT_FALSE; break;
	}
	PropVariantClear(&value);
	return result;
}

uint32_t ReadBackgroundColor(IWICImagingFactory& factory, IWICBitmapDecoder& decoder, IWICMetadataQueryReader& metadata) {
	if (!ReadMetadata(&metadata, L"/logscrdesc/GlobalColorTableFlag")) return 0;
	ComPtr<IWICPalette> palette;
	WICColor colors[256];
	UINT colorCount = 0;
	const UINT index = ReadMetadata(&metadata, L"/logscrdesc/BackgroundColorIndex");
	if (FAILED(factory.CreatePalette(palette.GetAddressOf())) || FAILED(decoder.CopyPalette(palette.Get())) ||
		FAILED(palette->GetColors(256, colors, &colorCount)) || index >= colorCount) return 0;
	return colors[index];
}

std::optional<GifImage> DecodeGif(const std::wstring& path, bool firstFrameOnly) {
	bool isWic2 = false;
	auto* factory = DirectX::GetWICFactory(isWic2);
	ComPtr<IWICBitmapDecoder> decoder;
	if (!factory || FAILED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf()))) return std::nullopt;

	GUID container{};
	UINT frameCount = 0;
	ComPtr<IWICMetadataQueryReader> metadata;
	if (FAILED(decoder->GetContainerFormat(&container)) || container != GUID_ContainerFormatGif ||
		FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0 ||
		FAILED(decoder->GetMetadataQueryReader(metadata.GetAddressOf()))) return std::nullopt;
	if (firstFrameOnly) frameCount = 1;

	const UINT width = ReadMetadata(metadata.Get(), L"/logscrdesc/Width");
	const UINT height = ReadMetadata(metadata.Get(), L"/logscrdesc/Height");
	if (!width || !height || width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) return std::nullopt;

	const size_t pixelCount = static_cast<size_t>(width) * height;
	// 展開後のサイズと、コマごとに作成するGPUテクスチャの最小割り当てサイズを考慮する。
	const size_t frameBytes = std::max(pixelCount * sizeof(uint32_t), static_cast<size_t>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT));
	if (frameCount > kMaxFrameTextureBytes / frameBytes) return std::nullopt;

	GifImage gif{ width, height, {} };
	gif.frames.reserve(frameCount);
	uint32_t background = ReadBackgroundColor(*factory, *decoder.Get(), *metadata.Get());
	std::vector<uint32_t> canvas(pixelCount);
	std::vector<uint32_t> previous;
	std::vector<uint32_t> pixels;
	for (UINT index = 0; index < frameCount; ++index) {
		ComPtr<IWICBitmapFrameDecode> frame;
		ComPtr<IWICMetadataQueryReader> frameMetadata;
		UINT frameWidth = 0, frameHeight = 0;
		if (FAILED(decoder->GetFrame(index, frame.GetAddressOf())) || FAILED(frame->GetSize(&frameWidth, &frameHeight)) ||
			FAILED(frame->GetMetadataQueryReader(frameMetadata.GetAddressOf()))) return std::nullopt;

		const UINT left = ReadMetadata(frameMetadata.Get(), L"/imgdesc/Left");
		const UINT top = ReadMetadata(frameMetadata.Get(), L"/imgdesc/Top");
		if (!frameWidth || !frameHeight || left >= width || top >= height) return std::nullopt;
		const UINT drawWidth = std::min(frameWidth, width - left);
		const UINT drawHeight = std::min(frameHeight, height - top);
		const WICRect source{ 0, 0, static_cast<INT>(drawWidth), static_cast<INT>(drawHeight) };

		ComPtr<IWICFormatConverter> converter;
		pixels.resize(static_cast<size_t>(drawWidth) * drawHeight);
		if (FAILED(factory->CreateFormatConverter(converter.GetAddressOf())) ||
			FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone,
				nullptr, 0.0, WICBitmapPaletteTypeCustom)) ||
			FAILED(converter->CopyPixels(&source, drawWidth * static_cast<UINT>(sizeof(uint32_t)),
				static_cast<UINT>(pixels.size() * sizeof(uint32_t)), reinterpret_cast<BYTE*>(pixels.data())))) return std::nullopt;

		const bool transparent = ReadMetadata(frameMetadata.Get(), L"/grctlext/TransparencyFlag") != 0;
		const UINT disposal = ReadMetadata(frameMetadata.Get(), L"/grctlext/Disposal");
		if (index == 0) {
			if (transparent) background = 0;
			std::ranges::fill(canvas, background);
		}
		if (disposal == kRestorePrevious) previous = canvas;

		// 透過画素は下のコマを残し、指定位置に今回のコマを重ねる。
		for (UINT y = 0; y < drawHeight; ++y) {
			const auto* row = pixels.data() + static_cast<size_t>(y) * drawWidth;
			auto* dest = canvas.data() + static_cast<size_t>(top + y) * width + left;
			for (UINT x = 0; x < drawWidth; ++x) {
				if (row[x] >> 24) dest[x] = row[x];
			}
		}

		// GIFの遅延は1/100秒単位。0または未指定なら100ms表示する。
		const UINT delay = ReadMetadata(frameMetadata.Get(), L"/grctlext/Delay");
		gif.frames.push_back({ canvas, (delay ? delay : 10) / 100.0 });
		if (disposal == kRestorePrevious) {
			canvas.swap(previous);
		} else if (disposal == kRestoreBackground) {
			for (UINT y = 0; y < drawHeight; ++y) {
				std::fill_n(canvas.data() + static_cast<size_t>(top + y) * width + left, drawWidth, transparent ? 0 : background);
			}
		}
	}
	return gif;
}

NoEngine::TextureRef UploadFrame(const std::wstring& key, const GifImage& gif, const GifFrame& frame) {
	return NoEngine::TextureManager::CreateFromPixels(key, gif.width, gif.height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		frame.bgra.data(), static_cast<size_t>(gif.width) * sizeof(uint32_t));
}
}

namespace NoEngine {
struct GifTexturePlayer::Clip {
	struct Frame {
		TextureRef texture;
		double endSeconds;
	};
	std::vector<Frame> frames;
};

bool IsGifTexture(const std::filesystem::path& path) {
	return _wcsicmp(path.extension().c_str(), L".gif") == 0;
}

TextureRef LoadGifFirstFrame(const std::wstring& path) {
	const auto key = std::filesystem::path(path).lexically_normal().native();
	const auto gif = DecodeGif(key, true);
	if (gif) return UploadFrame(L"GifTexture:" + key + L":0", *gif, gif->frames.front());
	LogWarning("GIFを読み込めませんでした: " + ConvertString(path));
	return {};
}

bool GifTexturePlayer::Load(const std::wstring& path) {
	static std::unordered_map<std::wstring, std::weak_ptr<const Clip>> cache;
	static std::mutex mutex;
	const auto key = std::filesystem::path(path).lexically_normal().native();
	std::lock_guard lock(mutex);
	auto& entry = cache[key];
	time_ = 0.0;
	if (auto cached = entry.lock()) {
		clip_ = std::move(cached);
		return true;
	}

	const auto gif = DecodeGif(key, false);
	if (!gif) {
		cache.erase(key);
		clip_.reset();
		LogWarning("GIFを読み込めませんでした: " + ConvertString(path));
		return false;
	}

	auto clip = std::make_shared<Clip>();
	clip->frames.reserve(gif->frames.size());
	double endSeconds = 0.0;
	for (const auto& frame : gif->frames) {
		endSeconds += frame.durationSeconds;
		const auto frameKey = L"GifTexture:" + key + L":" + std::to_wstring(clip->frames.size());
		clip->frames.push_back({ UploadFrame(frameKey, *gif, frame), endSeconds });
	}
	clip_ = clip;
	entry = clip;
	return true;
}

TextureRef GifTexturePlayer::GetFrame() const {
	if (!clip_) return {};
	const auto frame = std::ranges::upper_bound(clip_->frames, time_, {}, &Clip::Frame::endSeconds);
	return frame->texture;
}

void GifTexturePlayer::Advance(float deltaTime) {
	if (!clip_ || clip_->frames.size() < 2 || !std::isfinite(deltaTime) || deltaTime <= 0.f) return;
	time_ = std::fmod(time_ + deltaTime, clip_->frames.back().endSeconds);
}
}
