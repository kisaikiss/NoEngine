#include "FontManager.h"

#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Utilities/FileUtilities.h"
#include "engine/Assets/Texture/TextureManager.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "externals/stb/stb_truetype.h"

// ---------------------------------------------------------------------------
// フォントはTTF/OTFファイルをそのまま読み込み、実際に描画で使われた文字だけをその場でSDFラスタライズして
// アトラスへ追記していく(遅延ベイク)。ひらがな/カタカナ/漢字を含む全コードポイントを事前に総ベイクすると
// アトラスサイズ・ロード時間の両方が非現実的になるため、この方式を採っている。
// ---------------------------------------------------------------------------

namespace NoEngine {
using namespace Utilities;

namespace {
// ベイク時の基準ピクセル高さ。GlyphInfo::size / bearing / lineHeight / ascent はこのスケールでの値。
constexpr float kBakePixelHeight = 48.0f;

// SDF生成パラメータ(stb_truetype準拠)。
constexpr int kSdfPadding = 4;
constexpr uint8_t kSdfOnEdgeValue = 128;
constexpr float kSdfPixelDistScale = 32.0f;

// アトラス内グリフ間の余白(px)。
constexpr int kAtlasPadding = 1;
}

// stbtt_fontinfoの完全な定義が必要なため、Font::~Font()の定義はここに置く。
Font::~Font() = default;

bool Font::EnsureGlyphsBaked(const std::u32string& codepoints) {
	if (!fontInfo_) return false;

	bool addedAny = false;

	for (char32_t c : codepoints) {
		if (glyphs_.find(c) != glyphs_.end()) continue; // 既にベイク済み
		if (atlasFull_) continue; // アトラスが尽きている場合は以後スキップ(既存分のみで描画を続ける)

		// フォントにグリフが存在しない場合はnotdef(0)。空グリフとして登録し以後の探索コストを避ける。
		int glyphIndex = stbtt_FindGlyphIndex(fontInfo_.get(), static_cast<int>(c));
		if (glyphIndex == 0) {
			glyphs_[c] = GlyphInfo{};
			continue;
		}

		int advanceRaw = 0, lsbRaw = 0;
		stbtt_GetGlyphHMetrics(fontInfo_.get(), glyphIndex, &advanceRaw, &lsbRaw);

		GlyphInfo info{};
		info.advance = static_cast<float>(advanceRaw) * bakeScale_;

		int w = 0, h = 0, xoff = 0, yoff = 0;
		uint8_t* bitmap = stbtt_GetGlyphSDF(
			fontInfo_.get(), bakeScale_, glyphIndex,
			kSdfPadding, kSdfOnEdgeValue, kSdfPixelDistScale,
			&w, &h, &xoff, &yoff);

		if (bitmap && w > 0 && h > 0) {
			// シェルフパッキング: 現在の行に入らなければ次の行へ
			if (packCursorX_ + w + kAtlasPadding > kAtlasWidth) {
				packCursorX_ = kAtlasPadding;
				packCursorY_ += packRowHeight_ + kAtlasPadding;
				packRowHeight_ = 0;
			}
			if (packCursorY_ + h + kAtlasPadding > kAtlasHeight) {
				// アトラスが尽きた。これ以降の新規グリフはベイクしない。
				stbtt_FreeSDF(bitmap, nullptr);
				atlasFull_ = true;
				LogWarning("Font: SDF atlas is full. Some glyphs will not be rendered.");
				glyphs_[c] = GlyphInfo{};
				continue;
			}

			for (int y = 0; y < h; ++y) {
				uint8_t* dst = atlasPixels_.data() + static_cast<size_t>(packCursorY_ + y) * kAtlasWidth + packCursorX_;
				const uint8_t* src = bitmap + static_cast<size_t>(y) * w;
				std::memcpy(dst, src, w);
			}

			info.uv.x = static_cast<float>(packCursorX_) / static_cast<float>(kAtlasWidth);
			info.uv.y = static_cast<float>(packCursorY_) / static_cast<float>(kAtlasHeight);
			info.uv.width = static_cast<float>(w) / static_cast<float>(kAtlasWidth);
			info.uv.height = static_cast<float>(h) / static_cast<float>(kAtlasHeight);
			info.size = { static_cast<float>(w), static_cast<float>(h) };
			info.bearing = { static_cast<float>(xoff), static_cast<float>(yoff) };

			packCursorX_ += w + kAtlasPadding;
			packRowHeight_ = std::max(packRowHeight_, h);

			stbtt_FreeSDF(bitmap, nullptr);
			addedAny = true;
		}

		glyphs_[c] = info;
	}

	if (addedAny) {
		TextureManager::UpdatePixels(atlasTextureKey_, kAtlasWidth, kAtlasHeight,
			DXGI_FORMAT_R8_UNORM, atlasPixels_.data(), static_cast<size_t>(kAtlasWidth));
	}

	return addedAny;
}

#pragma region ManagedFont
class ManagedFont : public Font {
	friend class FontRef;
	friend class FontManager;
public:
	ManagedFont(const wstring& fileName) : isValid_(false), isLoading_(true), referenceCount_(0) {
		mapKey_ = fileName;
	}

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

void ManagedFont::CreateFromMemory(ByteArray ba) {
	if (!ba || ba->empty()) {
		LogWarning("FontManager: font file is empty. key : " + ConvertString(mapKey_));
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	fontData_ = ba; // stbtt_fontinfoが指し続けるため、Fontの生存期間中は保持する
	fontInfo_ = std::make_unique<stbtt_fontinfo>();

	int fontOffset = stbtt_GetFontOffsetForIndex(fontData_->data(), 0);
	if (fontOffset < 0 || !stbtt_InitFont(fontInfo_.get(), fontData_->data(), fontOffset)) {
		LogError("FontManager: failed to parse ttf/otf. key : " + ConvertString(mapKey_));
		fontInfo_.reset();
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	bakeScale_ = stbtt_ScaleForPixelHeight(fontInfo_.get(), kBakePixelHeight);

	int ascentRaw = 0, descentRaw = 0, lineGapRaw = 0;
	stbtt_GetFontVMetrics(fontInfo_.get(), &ascentRaw, &descentRaw, &lineGapRaw);
	ascent_ = static_cast<float>(ascentRaw) * bakeScale_;
	lineHeight_ = static_cast<float>(ascentRaw - descentRaw + lineGapRaw) * bakeScale_;
	distanceRange_ = 127.0f / kSdfPixelDistScale;

	// グリフは事前ベイクせず、EnsureGlyphsBaked()を通じて実際に使われた文字だけを随時追加する。
	atlasPixels_.assign(static_cast<size_t>(kAtlasWidth) * kAtlasHeight, 0);
	packCursorX_ = kAtlasPadding;
	packCursorY_ = kAtlasPadding;
	packRowHeight_ = 0;
	atlasFull_ = false;
	glyphs_.clear();

	atlasTextureKey_ = L"FontAtlas:" + mapKey_;
	atlasTexture_ = TextureManager::CreateFromPixels(
		atlasTextureKey_, kAtlasWidth, kAtlasHeight, DXGI_FORMAT_R8_UNORM,
		atlasPixels_.data(), static_cast<size_t>(kAtlasWidth));

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
Font* FontRef::Get(void) {
	return ref_;
}
const Font* FontRef::operator->(void) const {
	assert(ref_ != nullptr);
	return ref_;
}
#pragma endregion
}
