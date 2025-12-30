#include "stdafx.h"
#include "FileUtilities.h"
#include "Conversion/ConvertString.h"

namespace NoEngine {
namespace Utilities {
ByteArray NullFile = make_shared<vector<uint8_t> >(vector<uint8_t>());

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

ByteArray ReadFileHelper(const std::wstring& fileName) {
    if (!std::filesystem::exists(fileName))
        return NullFile;

    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    if (!file)
        return NullFile;

    auto fileSize = std::filesystem::file_size(fileName);

    ByteArray byteArray = std::make_shared<std::vector<uint8_t>>(fileSize);
    file.read((char*)byteArray->data(), byteArray->size());
    file.close();

    return byteArray;
}

ByteArray ReadFileSync(const std::wstring& fileName) {
    return ReadFileHelper(*(std::make_shared<std::wstring>(fileName)));
}
}
}