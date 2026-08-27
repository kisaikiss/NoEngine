#include "FontManager.h"

#include <sstream>
#include <algorithm>
#include <cstring>
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Utilities/FileUtilities.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "externals/stb/stb_truetype.h"

// ---------------------------------------------------------------------------
// フォントはTTF/OTFファイルをそのまま読み込み、ロード時にSDFアトラスへベイクする。
// パース・ラスタライズ・SDF生成はすべてstb_truetype(externals/stb/stb_truetype.h)に委譲し、
// 生成したアトラス画像はTextureManager::CreateFromPixels()でGPUテクスチャ化する
// (ファイルを経由しないため、LoadCovertTexture系のDDS変換パイプラインは通らない)。
// ---------------------------------------------------------------------------

namespace NoEngine {
using namespace Utilities;

#pragma region ManagedFont
class ManagedFont : public Font {
	friend class FontRef;
	friend class FontManager;
public:
	ManagedFont(const wstring& fileName) : mapKey_(fileName), isValid_(false), isLoading_(true), referenceCount_(0) {}

	void WaitForLoad(void) const {
		while ((volatile bool&)isLoading_)
			this_thread::yield();
	}
	void CreateFromMemory(ByteArray memory);

private:
	bool IsValid(void) const { return isValid_; }
	void Unload() {
		FontManager::DestroyFont(mapKey_);
	}

	std::wstring mapKey_;
	bool isValid_;
	bool isLoading_;
	size_t referenceCount_;
};

namespace {
// ベイク時の基準ピクセル高さ。GlyphInfo::size / bearing / lineHeight / ascent は
// すべてこのスケールでの値になる(TextComponent側のscaleでこれを基準に拡縮する想定)。
// ToDo: LoadFontFileの引数として外から指定できるようにする
constexpr float kBakePixelHeight = 48.0f;

// SDF生成パラメータ。onEdgeValueを跨いだ1pxの距離変化がpixelDistScaleだけ8bit値を変化させる
// (stb_truetype準拠)。distanceRange_はTextピクセルシェーダのsmoothstep幅計算に使う
constexpr int kSdfPadding = 4;
constexpr uint8_t kSdfOnEdgeValue = 128;
constexpr float kSdfPixelDistScale = 32.0f;

// アトラスの幅(px)。ASCII可視文字程度なら十分だが、収録コードポイントを増やす場合は広げる。
constexpr int kAtlasWidth = 512;
constexpr int kAtlasPadding = 1;

// ベイクするコードポイント範囲(ASCII可視文字のみ)。
// 日本語等を表示したい場合はここへ範囲を追加する(アトラス幅・パッキングも合わせて見直す)。
constexpr char32_t kFirstCodepoint = 0x20;
constexpr char32_t kLastCodepoint = 0x7E;

struct RawGlyph {
	char32_t codepoint = 0;
	std::vector<uint8_t> pixels; // SDFビットマップ(1チャンネル, width*height)
	int width = 0;
	int height = 0;
	int bearingX = 0; // カーソル位置からビットマップ左上までのオフセット(px)
	int bearingY = 0;
	float advance = 0.0f;
	int atlasX = 0;   // パッキング後のアトラス内座標(左上)
	int atlasY = 0;
};
}

void ManagedFont::CreateFromMemory(ByteArray ba) {
	if (!ba || ba->empty()) {
		LogWarning("FontManager: font file is empty. key : " + ConvertString(mapKey_));
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	stbtt_fontinfo fontInfo{};
	int fontOffset = stbtt_GetFontOffsetForIndex(ba->data(), 0);
	if (fontOffset < 0 || !stbtt_InitFont(&fontInfo, ba->data(), fontOffset)) {
		LogError("FontManager: failed to parse ttf/otf. key : " + ConvertString(mapKey_));
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	const float scale = stbtt_ScaleForPixelHeight(&fontInfo, kBakePixelHeight);

	int ascentRaw = 0, descentRaw = 0, lineGapRaw = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascentRaw, &descentRaw, &lineGapRaw);
	ascent_ = static_cast<float>(ascentRaw) * scale;
	lineHeight_ = static_cast<float>(ascentRaw - descentRaw + lineGapRaw) * scale;
	// 127階調(onEdgeValueからの片側幅)をpixelDistScaleで割ると、SDFが表現できる最大距離(px)になる
	distanceRange_ = 127.0f / kSdfPixelDistScale;

	// 1. 全グリフをラスタライズ(SDF化)する
	std::vector<RawGlyph> rawGlyphs;
	rawGlyphs.reserve(kLastCodepoint - kFirstCodepoint + 1);

	for (char32_t c = kFirstCodepoint; c <= kLastCodepoint; ++c) {
		RawGlyph g;
		g.codepoint = c;

		int advanceRaw = 0, lsbRaw = 0;
		stbtt_GetCodepointHMetrics(&fontInfo, static_cast<int>(c), &advanceRaw, &lsbRaw);
		g.advance = static_cast<float>(advanceRaw) * scale;

		int w = 0, h = 0, xoff = 0, yoff = 0;
		uint8_t* bitmap = stbtt_GetCodepointSDF(
			&fontInfo, scale, static_cast<int>(c),
			kSdfPadding, kSdfOnEdgeValue, kSdfPixelDistScale,
			&w, &h, &xoff, &yoff);

		if (bitmap && w > 0 && h > 0) {
			g.width = w;
			g.height = h;
			g.bearingX = xoff;
			g.bearingY = yoff;
			g.pixels.assign(bitmap, bitmap + static_cast<size_t>(w) * h);
			stbtt_FreeSDF(bitmap, nullptr);
		}

		rawGlyphs.push_back(std::move(g));
	}

	// 2. シェルフ(行送り)方式でアトラスへパッキングする。
	//    フォント1個・ASCII分程度の少数グリフを想定した簡易実装。
	int cursorX = kAtlasPadding, cursorY = kAtlasPadding, rowHeight = 0;
	for (auto& g : rawGlyphs) {
		if (g.width == 0 || g.height == 0) continue;

		if (cursorX + g.width + kAtlasPadding > kAtlasWidth) {
			cursorX = kAtlasPadding;
			cursorY += rowHeight + kAtlasPadding;
			rowHeight = 0;
		}
		g.atlasX = cursorX;
		g.atlasY = cursorY;
		cursorX += g.width + kAtlasPadding;
		rowHeight = std::max(rowHeight, g.height);
	}
	const int atlasHeight = std::max(1, cursorY + rowHeight + kAtlasPadding);

	// 3. アトラスピクセルバッファへ書き出す(1チャンネル、背景は完全に外側=0)
	std::vector<uint8_t> atlasPixels(static_cast<size_t>(kAtlasWidth) * atlasHeight, 0);
	for (auto& g : rawGlyphs) {
		if (g.width == 0 || g.height == 0) continue;
		for (int y = 0; y < g.height; ++y) {
			uint8_t* dst = atlasPixels.data() + static_cast<size_t>(g.atlasY + y) * kAtlasWidth + g.atlasX;
			const uint8_t* src = g.pixels.data() + static_cast<size_t>(y) * g.width;
			std::memcpy(dst, src, g.width);
		}
	}

	// 4. GlyphInfoを構築する
	glyphs_.clear();
	for (auto& g : rawGlyphs) {
		GlyphInfo info{};
		info.advance = g.advance;
		if (g.width > 0 && g.height > 0) {
			info.uv.x = static_cast<float>(g.atlasX) / static_cast<float>(kAtlasWidth);
			info.uv.y = static_cast<float>(g.atlasY) / static_cast<float>(atlasHeight);
			info.uv.width = static_cast<float>(g.width) / static_cast<float>(kAtlasWidth);
			info.uv.height = static_cast<float>(g.height) / static_cast<float>(atlasHeight);
			info.size = { static_cast<float>(g.width), static_cast<float>(g.height) };
			info.bearing = { static_cast<float>(g.bearingX), static_cast<float>(g.bearingY) };
		}
		glyphs_[g.codepoint] = info;
	}

	// 5. アトラスをGPUテクスチャ化する(ファイルを経由しないためCreateFromPixelsを使う)
	atlasTexture_ = TextureManager::CreateFromPixels(
		L"FontAtlas:" + mapKey_, kAtlasWidth, atlasHeight, DXGI_FORMAT_R8_UNORM,
		atlasPixels.data(), static_cast<size_t>(kAtlasWidth));

	isValid_ = atlasTexture_.IsValid();
	isLoading_ = false;
}
#pragma endregion

#pragma region FontManager
namespace {
wstring sRootPath = L"";
unordered_map<wstring, unique_ptr<ManagedFont>> sFontCache;
mutex sMutex;
}

void FontManager::Initialize(const std::wstring& FontRoot) {
	sRootPath = FontRoot;
}
void FontManager::Shutdown() {
	sFontCache.clear();
}
void FontManager::DestroyFont(std::wstring mapKey) {
	lock_guard<mutex> Guard(sMutex);

	auto iter = sFontCache.find(mapKey);
	if (iter != sFontCache.end())
		sFontCache.erase(iter);
}

FontRef FontManager::LoadFontFile(const std::wstring& filePath) {
	return FindOrLoadFont(filePath);
}
FontRef FontManager::LoadFontFile(const std::string& filePath) {
	return LoadFontFile(ConvertString(filePath));
}

ManagedFont* FontManager::FindOrLoadFont(const std::wstring& fileName) {
	ManagedFont* font = nullptr;

	{
		std::lock_guard<std::mutex> Guard(sMutex);

		auto iter = sFontCache.find(fileName);
		if (iter != sFontCache.end()) {
			font = iter->second.get();
			font->WaitForLoad();
			return font;
		} else {
			font = new ManagedFont(fileName);
			sFontCache[fileName].reset(font);
		}
	}

	ByteArray ba = ReadFileSync(sRootPath + fileName);
	font->CreateFromMemory(ba);

	return font;
}
#pragma endregion

#pragma region FontRef
FontRef::FontRef(const FontRef& ref) : ref_(ref.ref_) {
	if (ref_ != nullptr) {
		++ref_->referenceCount_;
	}
}
FontRef::FontRef(ManagedFont* font) : ref_(font) {
	if (ref_ != nullptr) {
		++ref_->referenceCount_;
	}
}
FontRef::~FontRef() {
	if (ref_ != nullptr && --ref_->referenceCount_ == 0)
		ref_->Unload();
}
void FontRef::operator= (std::nullptr_t) {
	if (ref_ != nullptr)
		--ref_->referenceCount_;

	ref_ = nullptr;
}
void FontRef::operator= (FontRef& rhs) {
	if (ref_ != nullptr)
		--ref_->referenceCount_;

	ref_ = rhs.ref_;

	if (ref_ != nullptr)
		++ref_->referenceCount_;
}
FontRef& FontRef::operator=(const FontRef& rhs) {
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
FontRef& FontRef::operator=(FontRef&& rhs) noexcept {
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
bool FontRef::IsValid() const {
	return ref_ && ref_->IsValid();
}
const Font* FontRef::Get(void) const {
	return ref_;
}
const Font* FontRef::operator->(void) const {
	assert(ref_ != nullptr);
	return ref_;
}
#pragma endregion
}
