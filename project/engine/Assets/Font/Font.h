#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {

// SDFフォントアトラス上の1グリフ分の情報
struct GlyphInfo {
	Rect uv;         // アトラス内UV矩形 (0-1正規化)
	Math::Vector2 size;     // クアッドサイズ(ピクセル, フォントの基準サイズ時)
	Math::Vector2 bearing;  // カーソル位置からクアッド左上までのオフセット
	float advance = 0.f;    // 次の文字カーソルへの送り幅
};

// フォント1つ分のアトラス+メトリクスの公開インターフェース。
// Texture基底クラスと同じ役割で、実際のロード処理はManagedFont(FontManager.cpp内)が担う。
class Font {
public:
	virtual ~Font() = default;

	const GlyphInfo* GetGlyph(char32_t codepoint) const {
		auto it = glyphs_.find(codepoint);
		return it != glyphs_.end() ? &it->second : nullptr;
	}

	TextureRef GetAtlasTexture() const { return atlasTexture_; }
	float GetLineHeight() const { return lineHeight_; }
	float GetAscent() const { return ascent_; }
	// SDFのスプレッド幅(生成時のピクセル単位)。ピクセルシェーダのsmoothstep幅算出に使う
	float GetDistanceRange() const { return distanceRange_; }

protected:
	std::unordered_map<char32_t, GlyphInfo> glyphs_;
	TextureRef atlasTexture_;
	float lineHeight_ = 0.f;
	float ascent_ = 0.f;
	float distanceRange_ = 4.f;
};

}

