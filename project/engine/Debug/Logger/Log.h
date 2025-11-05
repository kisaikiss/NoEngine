#pragma once
namespace NoEngine
{
enum class VerbosityLevel {
	kCritical = 0,	// すぐに直さなければならない(assert)
	kError,			// これが出たらちゃんと直して欲しい
	kWarning,		// 良くないことが起こりうる
	kInfo,			// 何が起こっているか確認する
	kDebug,			// 開発用
};

namespace Log {
	void Initialize();
	void SetVerbosityLevel(VerbosityLevel verbosityLevel);
	void DebugPrint(const std::string& message, VerbosityLevel verbosityLevel = VerbosityLevel::kDebug);
};
}

