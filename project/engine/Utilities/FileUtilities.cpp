#include "stdafx.h"
#include "FileUtilities.h"
#include "Conversion/ConvertString.h"

namespace NoEngine {
namespace Utilities {
std::wstring LoadFileAsString(const std::wstring& path) {
    // ファイルを開きます。（バイナリモード + 位置移動可能）
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        return L"";

    // ファイル末尾に移動してサイズを取得します。
    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // バッファを確保します。
    std::string buffer(size, '\0');

    // ファイル内容を読み込みます。
    ifs.read(buffer.data(), size);
    
    return ConvertString(buffer);
}
}
}