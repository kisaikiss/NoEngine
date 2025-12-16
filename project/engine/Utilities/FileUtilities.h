#pragma once
namespace NoEngine {
namespace Utilities {
/// <summary>
/// ファイルの中身をwstring型で返します。
/// </summary>
/// <param name="path">ファイルパス</param>
/// <returns>ファイルの中身</returns>
std::wstring LoadFileAsString(const std::wstring& path);

}
}