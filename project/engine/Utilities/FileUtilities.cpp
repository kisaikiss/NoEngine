#include "stdafx.h"
#include "FileUtilities.h"
#include "Conversion/ConvertString.h"

namespace NoEngine {
	namespace Utilities
	{
		ByteArray NullFile = make_shared<vector<uint8_t> >(vector<uint8_t>());

		std::wstring LoadFileAsString(const std::wstring& path)
		{
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

		ByteArray ReadFileHelper(const std::wstring& fileName)
		{
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

		ByteArray ReadFileSync(const std::wstring& fileName)
		{
			return ReadFileHelper(*(std::make_shared<std::wstring>(fileName)));
		}
		std::string ToLower(const std::string& str)
		{
			std::string lower_case = str;
			std::locale loc;
			for (char& s : lower_case)
				s = std::tolower(s, loc);
			return lower_case;
		}
		std::wstring ToLower(const std::wstring& str)
		{
			std::wstring lower_case = str;
			std::locale loc;
			for (wchar_t& s : lower_case)
				s = std::tolower(s, loc);
			return lower_case;
		}
		std::string RemoveBasePath(const std::string& filePath)
		{
			size_t lastSlash;
			if ((lastSlash = filePath.rfind('/')) != std::string::npos)
				return filePath.substr(lastSlash + 1, std::string::npos);
			else if ((lastSlash = filePath.rfind('\\')) != std::string::npos)
				return filePath.substr(lastSlash + 1, std::string::npos);
			else
				return filePath;
		}

		std::wstring RemoveBasePath(const std::wstring& filePath)
		{
			size_t lastSlash;
			if ((lastSlash = filePath.rfind(L'/')) != std::string::npos)
				return filePath.substr(lastSlash + 1, std::string::npos);
			else if ((lastSlash = filePath.rfind(L'\\')) != std::string::npos)
				return filePath.substr(lastSlash + 1, std::string::npos);
			else
				return filePath;
		}

		std::string GetFileExtension(const std::string& filePath)
		{
			std::string fileName = RemoveBasePath(filePath);
			size_t extOffset = fileName.rfind('.');
			if (extOffset == std::wstring::npos)
				return "";

			return fileName.substr(extOffset + 1);
		}

		std::wstring GetFileExtension(const std::wstring& filePath)
		{
			std::wstring fileName = RemoveBasePath(filePath);
			size_t extOffset = fileName.rfind(L'.');
			if (extOffset == std::wstring::npos)
				return L"";

			return fileName.substr(extOffset + 1);
		}

		std::string GetBasePath(const std::string& filePath)
		{
			size_t lastSlash;
			if ((lastSlash = filePath.rfind('/')) != std::string::npos)
				return filePath.substr(0, lastSlash + 1);
			else if ((lastSlash = filePath.rfind('\\')) != std::string::npos)
				return filePath.substr(0, lastSlash + 1);
			else
				return "";
		}

		std::wstring GetBasePath(const std::wstring& filePath)
		{
			size_t lastSlash;
			if ((lastSlash = filePath.rfind(L'/')) != std::wstring::npos)
				return filePath.substr(0, lastSlash + 1);
			else if ((lastSlash = filePath.rfind(L'\\')) != std::wstring::npos)
				return filePath.substr(0, lastSlash + 1);
			else
				return L"";
		}

		std::string RemoveExtension(const std::string& filePath)
		{
			return filePath.substr(0, filePath.rfind("."));
		}

		std::wstring RemoveExtension(const std::wstring& filePath)
		{
			return filePath.substr(0, filePath.rfind(L"."));
		}

		std::wstring GetDirectoryFromPath(const std::wstring& fullPath)
		{
			size_t pos = fullPath.find_last_of(L"/\\");
			return (pos != std::wstring::npos) ? fullPath.substr(0, pos + 1) : L"";
		}
	}
}