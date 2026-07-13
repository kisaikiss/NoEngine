#include "FontManager.h"

#include <sstream>
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Utilities/FileUtilities.h"

// ---------------------------------------------------------------------------
// グリフメタデータファイルの書式(テキスト, 1行1トークン列):
//
//   LINEHEIGHT <float>              改行時の行送り量(ピクセル, フォント基準サイズ時)
//   ASCENT <float>                  ベースラインから上端までの距離
//   DISTANCERANGE <float>           SDF生成時のスプレッド幅(ピクセル)
//   ATLAS <相対パス>                 アトラステクスチャ(TextureManagerが解決するルート基準の相対パス)
//   GLYPH <codepoint(10進)> u=.. v=.. uw=.. vh=.. w=.. h=.. bx=.. by=.. adv=..
//     u,v,uw,vh : アトラス内UV矩形(0-1正規化, x,y,width,height)
//     w,h       : グリフクアッドのサイズ(ピクセル)
//     bx,by     : カーソル位置からクアッド左上までのオフセット(ベアリング)
//     adv       : 次グリフへのカーソル送り幅
//   # から始まる行、空行は無視
//
// msdf-atlas-gen等のJSON出力を使う場合はParseGlyphLine/CreateFromMemory内の
// パース部分のみ差し替えれば、ManagedFont/FontRef側の構造はそのまま流用できる。
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
	void ParseGlyphLine(std::istringstream& lineStream);

	std::wstring mapKey_;
	bool isValid_;
	bool isLoading_;
	size_t referenceCount_;
};

void ManagedFont::ParseGlyphLine(std::istringstream& lineStream) {
	uint32_t codepoint = 0;
	lineStream >> codepoint;

	GlyphInfo glyph{};
	std::string kv;
	while (lineStream >> kv) {
		size_t eq = kv.find('=');
		if (eq == std::string::npos) continue;

		std::string key = kv.substr(0, eq);
		float value = 0.f;
		try {
			value = std::stof(kv.substr(eq + 1));
		}
		catch (...) {
			continue; // 壊れたトークンはスキップ(このグリフだけ欠けさせて他は読み続ける)
		}

		if (key == "u") glyph.uv.x = value;
		else if (key == "v") glyph.uv.y = value;
		else if (key == "uw") glyph.uv.width = value;
		else if (key == "vh") glyph.uv.height = value;
		else if (key == "w") glyph.size.x = value;
		else if (key == "h") glyph.size.y = value;
		else if (key == "bx") glyph.bearing.x = value;
		else if (key == "by") glyph.bearing.y = value;
		else if (key == "adv") glyph.advance = value;
	}

	glyphs_[static_cast<char32_t>(codepoint)] = glyph;
}

void ManagedFont::CreateFromMemory(ByteArray ba) {
	if (!ba || ba->empty()) {
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	std::string content(reinterpret_cast<const char*>(ba->data()), ba->size());
	std::istringstream stream(content);
	std::string line;
	std::wstring atlasPath;

	while (std::getline(stream, line)) {
		if (line.empty() || line[0] == '#') continue;

		std::istringstream lineStream(line);
		std::string token;
		lineStream >> token;

		if (token == "LINEHEIGHT") {
			lineStream >> lineHeight_;
		} else if (token == "ASCENT") {
			lineStream >> ascent_;
		} else if (token == "DISTANCERANGE") {
			lineStream >> distanceRange_;
		} else if (token == "ATLAS") {
			std::string path;
			lineStream >> path;
			atlasPath = ConvertString(path);
		} else if (token == "GLYPH") {
			ParseGlyphLine(lineStream);
		}
		// 未知のトークンは将来の拡張用に無視する
	}

	if (atlasPath.empty()) {
		isValid_ = false;
		isLoading_ = false;
		return;
	}

	// SDFアトラスは色空間補正の対象ではないのでsRGB=falseで読む
	atlasPath = RemoveExtension(atlasPath) + L".png";
	atlasTexture_ = TextureManager::LoadCovertTexture(atlasPath, Graphics::eDefaultTexture::kMagenta2D, false);

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
			// 既存の管理フォントが見つかった場合、ロード完了を待ってから返す
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
