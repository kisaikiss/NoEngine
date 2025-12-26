#pragma once

namespace NoEngine {
namespace Utilities {
using namespace std;

typedef shared_ptr<vector<uint8_t> > ByteArray;
extern ByteArray NullFile;

/// <summary>
/// ファイルの中身をwstring型で返します。
/// </summary>
/// <param name="path">ファイルパス</param>
/// <returns>ファイルの中身</returns>
std::wstring LoadFileAsString(const std::wstring& path);

}
}