#pragma once
namespace NoEngine {
std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);

std::string FormatToString(DXGI_FORMAT fmt);

std::string GetHResultMessage(HRESULT hr);

// UTF-8バイト列をUnicodeコードポイント列にデコードする。
// 不正なバイト列や末尾で途切れた継続バイトはU+FFFD(置換文字)として扱う。
std::u32string Utf8ToCodepoints(const std::string& utf8);
}