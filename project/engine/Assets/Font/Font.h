#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Math/Types/Vector2.h"

// stb_truetypeの実体はFontManager.cppでのみinclude(STB_TRUETYPE_IMPLEMENTATION)するため、
// このヘッダでは前方宣言のみに留め、他の.cppにstb_truetypeを持ち込まないようにする。
struct stbtt_fontinfo;

namespace NoEngine {

struct UVRect {
	float x = 0.f, y = 0.f, width = 0.f, height = 0.f;
};

struct GlyphInfo {
	UVRect uv;
	Math::Vector2 size = { 0.f, 0.f };
	Math::Vector2 bearing = { 0.f, 0.f };
	float advance = 0.f;
};

class Font {
public:
	virtual ~Font(); // stbtt_fontinfoが不完全型のため、定義はFontManager.cpp側に置く

	const GlyphInfo* GetGlyph(char32_t codepoint) const {
		auto it = glyphs_.find(codepoint);
		return it != glyphs_.end() ? &it->second : nullptr;
	}
	float GetAscent(void) const { return ascent_; }
	float GetLineHeight(void) const { return lineHeight_; }
	float GetDistanceRange(void) const { return distanceRange_; }
	TextureRef GetAtlasTexture(void) const { return atlasTexture_; }

	// codepoints中の未ベイク文字をまとめてラスタライズし、アトラスへ追加する。
	// 1文字でも新規追加があればアトラス全体をGPUへ再アップロードする(部分アップロードは未実装)。
	// TextPassは描画前に必ず呼ぶこと。既にベイク済みの文字だけなら追加コストはハッシュマップ検索のみ。
	bool EnsureGlyphsBaked(const std::u32string& codepoints);

protected:
	std::unordered_map<char32_t, GlyphInfo> glyphs_;
	float ascent_ = 0.f;
	float lineHeight_ = 0.f;
	float distanceRange_ = 0.f;
	TextureRef atlasTexture_;

	// --- 動的ベイク用の永続状態 ---
	// stbtt_fontinfoはfontData_内部へのポインタを保持し続けるため、
	// Fontが生きている間はfontData_を解放してはいけない。
	std::shared_ptr<std::vector<uint8_t>> fontData_;
	std::unique_ptr<stbtt_fontinfo> fontInfo_;
	float bakeScale_ = 1.f;

	static constexpr int kAtlasWidth = 2048;
	static constexpr int kAtlasHeight = 2048;
	std::vector<uint8_t> atlasPixels_; // 再アップロード用にCPU側へ全体を保持しておく
	int packCursorX_ = 1;
	int packCursorY_ = 1;
	int packRowHeight_ = 0;
	bool atlasFull_ = false;

	std::wstring atlasTextureKey_;

	friend class ManagedFont;
};

}