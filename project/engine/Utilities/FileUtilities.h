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

ByteArray ReadFileSync(const std::wstring& fileName);
std::string ToLower(const std::string& str);
std::wstring ToLower(const std::wstring& str);
std::string RemoveBasePath(const std::string& str);
std::wstring RemoveBasePath(const std::wstring& str);
std::string GetFileExtension(const std::string& str);
std::wstring GetFileExtension(const std::wstring& str);
std::string GetBasePath(const std::string& filePath);
std::wstring GetBasePath(const std::wstring& filePath);
std::string RemoveExtension(const std::string& str);
std::wstring RemoveExtension(const std::wstring& str);
std::wstring GetDirectoryFromPath(const std::wstring& fullPath);

void SIMDMemCopy(void* __restrict Dest, const void* __restrict Source, size_t NumQuadwords);
void SIMDMemFill(void* __restrict Dest, __m128 FillVector, size_t NumQuadwords);
}
}