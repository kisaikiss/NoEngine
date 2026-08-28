#pragma once
#include "Font.h"

namespace NoEngine {
class ManagedFont;
class FontRef;

class FontManager {
public:
	static void Initialize(const std::wstring& FontRoot);
	static void Shutdown(void);
	static void DestroyFont(std::wstring mapKey);

	// filePath: グリフメタデータファイル(テキスト形式, 書式は FontManager.cpp 冒頭のコメント参照)
	// メタデータ内のATLAS行に書かれたパスを使い、アトラステクスチャはTextureManager経由でロードされる
	static FontRef LoadFontFile(const std::wstring& filePath);
	static FontRef LoadFontFile(const std::string& filePath);

private:
	static ManagedFont* FindOrLoadFont(const std::wstring& fileName);
};

class FontRef {
public:
	FontRef(const FontRef& ref);
	FontRef(ManagedFont* font = nullptr);
	~FontRef();

	void operator= (std::nullptr_t);
	void operator= (FontRef& rhs);
	FontRef& operator=(const FontRef& rhs);
	FontRef& operator=(FontRef&& rhs) noexcept;

	// これが有効なフォント(メタデータ+アトラスのロードに成功したもの)を指していることを確認します。
	bool IsValid() const;

	// フォント本体を取得します。クライアントは null ポインタを参照しない責任があります。
	Font* Get(void);
	const Font* operator->(void) const;

	bool operator<(const FontRef& other) const {
		return ref_ < other.ref_; // TextPassでフォントをまとめて描画するために使用します。
	}
	bool operator==(const FontRef& other) const {
		return ref_ == other.ref_;
	}

private:
	ManagedFont* ref_;
};
}
