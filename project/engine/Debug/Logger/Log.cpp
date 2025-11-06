#include "Log.h"
#include <dxgidebug.h>
#include <filesystem>
#include <fstream>
#include <chrono>

namespace NoEngine {
namespace {
VerbosityLevel sVerbosityLevel = VerbosityLevel::kDebug;
std::string sFilePath = "";
}

void Log::Initialize() {
	//ログのディレクトリを用意
	std::filesystem::create_directory("logs");
	//現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point < std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	sFilePath = std::string("logs/") + dateString + ".log";
	//ファイルを作って書き込み準備
	std::ofstream logStream(sFilePath);
}

void Log::SetVerbosityLevel(VerbosityLevel verbosityLevel) {
	sVerbosityLevel = verbosityLevel;
}

void Log::DebugPrint(const std::string& message, VerbosityLevel verbosityLevel) {
	if (sVerbosityLevel >= verbosityLevel) {
		std::ofstream logStream(sFilePath, std::ios::app);
		if (sVerbosityLevel >= VerbosityLevel::kDebug) {
			std::string logMessage = message + "\nverbosityLevel:" + std::to_string(static_cast<int>(verbosityLevel)) + "\n";
			logStream << logMessage << std::endl;
			OutputDebugStringA(logMessage.c_str());
		} else {
			logStream << message << std::endl;
			OutputDebugStringA(message.c_str());
		}
		
	}
}

}