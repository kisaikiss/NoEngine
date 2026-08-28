#include "ConvertString.h"

namespace NoEngine {
std::wstring ConvertString(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& str) {
    if (str.empty()) {
        return std::string();
    }

    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

std::string FormatToString(DXGI_FORMAT fmt) {
    switch (fmt) {
    case DXGI_FORMAT_R32_FLOAT: return "R32_FLOAT";
    case DXGI_FORMAT_R32G32_FLOAT: return "R32G32_FLOAT";
    case DXGI_FORMAT_R32G32B32_FLOAT: return "R32G32B32_FLOAT";
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return "R32G32B32A32_FLOAT";
    default: return "UNKNOWN";
    }
}

std::string GetHResultMessage(HRESULT hr) {
    return std::system_category().message(hr);
}

std::u32string Utf8ToCodepoints(const std::string& utf8) {
	std::u32string result;
	result.reserve(utf8.size());

	size_t i = 0;
	const size_t n = utf8.size();
	while (i < n) {
		const unsigned char c0 = static_cast<unsigned char>(utf8[i]);
		char32_t codepoint = 0;
		size_t extraBytes = 0;

		if ((c0 & 0x80) == 0x00) { codepoint = c0; extraBytes = 0; }             // 1byte (ASCII)
		else if ((c0 & 0xE0) == 0xC0) { codepoint = c0 & 0x1F; extraBytes = 1; } // 2byte
		else if ((c0 & 0xF0) == 0xE0) { codepoint = c0 & 0x0F; extraBytes = 2; } // 3byte (ひらがな/カタカナ/大半の漢字はここ)
		else if ((c0 & 0xF8) == 0xF0) { codepoint = c0 & 0x07; extraBytes = 3; } // 4byte (一部の異体字・絵文字等)
		else {
			result.push_back(0xFFFD); // 不正な先頭バイト
			++i;
			continue;
		}

		if (i + extraBytes >= n) {
			result.push_back(0xFFFD); // 末尾で継続バイトが不足
			break;
		}

		bool valid = true;
		for (size_t k = 1; k <= extraBytes; ++k) {
			const unsigned char cc = static_cast<unsigned char>(utf8[i + k]);
			if ((cc & 0xC0) != 0x80) { valid = false; break; } // 継続バイトは10xxxxxx以外不正
			codepoint = (codepoint << 6) | (cc & 0x3F);
		}

		if (!valid) {
			result.push_back(0xFFFD);
			++i;
			continue;
		}

		result.push_back(codepoint);
		i += extraBytes + 1;
	}

	return result;
}
}