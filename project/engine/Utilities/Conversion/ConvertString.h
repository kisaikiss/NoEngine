#pragma once
namespace NoEngine {
std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);

std::string FormatToString(DXGI_FORMAT fmt);
}