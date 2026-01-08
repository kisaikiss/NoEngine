#include"TexUtil.h"
#include"engine/Debug/Logger/Log.h"
#include "engine/Utilities/FileUtilities.h"
#include <externals/DirectXTex/DirectXTex.h>
#include <sstream>

#define GetFlag(f) ((Flags & f) != 0)

namespace NoEngine
{
	using namespace Utilities;
	using namespace DirectX;

	// wstringをstringに変換するヘルパー関数
	static std::string WStringToString(const std::wstring& wstr)
	{
		if (wstr.empty()) return std::string();
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string str(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
		return str;
	}

	void CompileTextureOnDemand(const std::wstring& originalFile, uint32_t flags)
	{
		std::wstring ddsFile = RemoveExtension(originalFile) + L".dds";

		bool srcFileExists = std::filesystem::exists(originalFile);
		bool ddsFileExists = std::filesystem::exists(ddsFile);

		if (!srcFileExists && !ddsFileExists)
		{
			Log::DebugPrint("Texture " + WStringToString(RemoveBasePath(originalFile)) + " is missing.\n", VerbosityLevel::kInfo);
			return;
		}

		std::filesystem::file_time_type srcLastWriteTime;
		std::filesystem::file_time_type ddsLastWriteTime;

		if (srcFileExists)
			srcLastWriteTime = std::filesystem::last_write_time(originalFile);

		if (ddsFileExists)
			ddsLastWriteTime = std::filesystem::last_write_time(ddsFile);

		if (!ddsFileExists || (srcFileExists && ddsLastWriteTime < srcLastWriteTime))
		{
			Log::DebugPrint("DDS mTexture " + WStringToString(RemoveBasePath(originalFile)) + " missing or older than source. Rebuilding.\n");
			ConvertToDDS(originalFile, flags);
		}
	}

	bool ConvertToDDS(const std::wstring& filePath, uint32_t Flags)
	{
		bool bInterpretAsSRGB = GetFlag(kSRGB);
		bool bPreserveAlpha = GetFlag(kPreserveAlpha);
		bool bContainsNormals = GetFlag(kNormalMap);
		bool bBumpMap = GetFlag(kBumpToNormal);
		bool bBlockCompress = GetFlag(kDefaultBC);
		bool bUseBestBC = GetFlag(kQualityBC);
		bool bFlipImage = GetFlag(kFlipVertical);

		assert(!bInterpretAsSRGB || !bContainsNormals);
		assert(!bPreserveAlpha || !bContainsNormals);

		std::string filePathStr = WStringToString(filePath);
		Log::DebugPrint("Converting file \"" + filePathStr + "\" to DDS.\n");

		// 拡張子をutf8（ascii）として取得する
		std::wstring ext = ToLower(GetFileExtension(filePath));

		// テクスチャ画像を読み込む
		TexMetadata info;
		std::unique_ptr<ScratchImage> image(new ScratchImage);

		bool isDDS = false;
		bool isHDR = false;
		if (ext == L"dds")
		{
			isDDS = true;
			HRESULT hr = LoadFromDDSFile(filePath.c_str(), DDS_FLAGS_NONE, &info, *image);
			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not load mTexture \"" << filePathStr << "\" (DDS: " << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
				return false;
			}
		}
		else if (ext == L"tga")
		{
			HRESULT hr = LoadFromTGAFile(filePath.c_str(), &info, *image);
			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not load mTexture \"" << filePathStr << "\" (TGA: " << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
				return false;
			}
		}
		else if (ext == L"hdr")
		{
			isHDR = true;
			HRESULT hr = LoadFromHDRFile(filePath.c_str(), &info, *image);
			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not load mTexture \"" << filePathStr << "\" (HDR: " << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
				return false;
			}
		}
		else
		{
			WIC_FLAGS wicFlags = WIC_FLAGS_NONE;

			HRESULT hr = LoadFromWICFile(filePath.c_str(), wicFlags, &info, *image);
			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not load mTexture \"" << filePathStr << "\" (WIC: " << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
				return false;
			}
		}

		if (info.width > 16384 || info.height > 16384)
		{
			std::ostringstream oss;
			oss << "Texture size (" << info.width << "," << info.height << ") too large for feature level 11.0 or later (16384) \"" << filePathStr << "\".\n";
			Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			return false;
		}

		if (bFlipImage)
		{
			std::unique_ptr<ScratchImage> timage(new ScratchImage);

			HRESULT hr = FlipRotate(image->GetImages()[0], TEX_FR_FLIP_VERTICAL, *timage);

			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not flip image \"" << filePathStr << "\" (" << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			}
			else
			{
				image.swap(timage);
			}

		}

		DXGI_FORMAT tformat;
		DXGI_FORMAT cformat;

		if (isHDR)
		{
			tformat = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
			cformat = bBlockCompress ? DXGI_FORMAT_BC6H_UF16 : DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		}
		else if (bBlockCompress)
		{
			tformat = bInterpretAsSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
			if (bUseBestBC)
				cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
			else if (bPreserveAlpha)
				cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
			else
				cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
		}
		else
		{
			cformat = tformat = bInterpretAsSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		if (bBumpMap)
		{
			std::unique_ptr<ScratchImage> timage(new ScratchImage);

			HRESULT hr = ComputeNormalMap(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
				CNMAP_CHANNEL_LUMINANCE, 10.0f, tformat, *timage);

			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not compute normal map for \"" << filePathStr << "\" (" << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			}
			else
			{
				image.swap(timage);
				info.format = tformat;
			}
		}
		else if (info.format != tformat)
		{
			std::unique_ptr<ScratchImage> timage(new ScratchImage);

			HRESULT hr = Convert(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
				tformat, TEX_FILTER_DEFAULT, 0.5f, *timage);

			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Could not convert \"" << filePathStr << "\" (" << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			}
			else
			{
				image.swap(timage);
				info.format = tformat;
			}
		}

		// ミップマップ
		if (info.mipLevels == 1)
		{
			std::unique_ptr<ScratchImage> timage(new ScratchImage);

			HRESULT hr = GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(), TEX_FILTER_DEFAULT, 0, *timage);

			if (FAILED(hr))
			{
				std::ostringstream oss;
				oss << "Failing generating mipmaps for \"" << filePathStr << "\" (WIC: " << hr << ").\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			}
			else
			{
				image.swap(timage);
			}
		}

		// ハンドル圧縮
		if (bBlockCompress)
		{
			if (info.width % 4 || info.height % 4)
			{
				std::ostringstream oss;
				oss << "Texture size (" << info.width << "," << info.height << ") not a multiple of 4 \"" << filePathStr << "\", so skipping compress\n";
				Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			}
			else
			{
				std::unique_ptr<ScratchImage> timage(new ScratchImage);

				HRESULT hr = Compress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), cformat, TEX_COMPRESS_DEFAULT, 0.5f, *timage);
				if (FAILED(hr))
				{
					std::ostringstream oss;
					oss << "Failing compressing \"" << filePathStr << "\" (WIC: " << hr << ").\n";
					Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
				}
				else
				{
					image.swap(timage);
				}
			}
		}

		// ファイル拡張子をDDSに変更する
		const std::wstring wDest = RemoveExtension(filePath) + L".dds";

		// DDSを保存
		HRESULT hr = SaveToDDSFile(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DDS_FLAGS_NONE, wDest.c_str());
		if (FAILED(hr))
		{
			std::ostringstream oss;
			oss << "Could not write mTexture to file \"" << WStringToString(wDest) << "\" (" << hr << ").\n";
			Log::DebugPrint(oss.str(), VerbosityLevel::kInfo);
			return false;
		}

		return true;
	}


}