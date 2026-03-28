#pragma once
#include "engine/NoEngine.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace CommentBout {
class GameAudio {
public:
	static void InitializeForCommentBout() {
		RegisterBGMClip("cb_test_bgm", L"resources/game/td_2304/Audio/BGM/titleBGM.mp3", 1.0f);
		RegisterSEClip("cb_test_se", L"resources/game/td_2304/Audio/SE/select.mp3", 1.0f);

		// BGM
		RegisterBGMClip("BGMInGame", L"resources/game/td_3105/Audio/BGM/InGameBGM01.mp3", 1.0f);
		RegisterBGMClip("BGMTitle",  L"resources/game/td_3105/Audio/BGM/TitleBGM01.mp3",  1.0f);

		// SE - 戦闘
		RegisterSEClip("SEBossDamage",   L"resources/game/td_3105/Audio/SE/BossDamage.mp3",   1.0f);
		RegisterSEClip("SEPlayerDamage", L"resources/game/td_3105/Audio/SE/PlayerDamage.mp3", 1.0f);
		RegisterSEClip("SEEnemyDamage",  L"resources/game/td_3105/Audio/SE/EnemyDamage.mp3",  1.0f);
		RegisterSEClip("SEEnemyShot",    L"resources/game/td_3105/Audio/SE/EnemyShot.mp3",    1.0f);
		RegisterSEClip("SEBossShot",     L"resources/game/td_3105/Audio/SE/BossShot.mp3",     1.0f);
		RegisterSEClip("SEPlayerPunch",  L"resources/game/td_3105/Audio/SE/PlayerPunch.mp3",  1.0f);

		// SE - 吹き出し
		RegisterSEClip("SESpeechBubbleL", L"resources/game/td_3105/Audio/SE/SpeechBubbleL.mp3", 1.0f);
		RegisterSEClip("SESpeechBubbleM", L"resources/game/td_3105/Audio/SE/SpeechBubbleM.mp3", 1.0f);
		RegisterSEClip("SESpeechBubbleS", L"resources/game/td_3105/Audio/SE/SpeechBubbleS.mp3", 1.0f);

		// SE - システム
		RegisterSEClip("SESystemDecision",   L"resources/game/td_3105/Audio/SE/SystemDecision.mp3",   10.0f);
		RegisterSEClip("SESystemMoveCursor", L"resources/game/td_3105/Audio/SE/SystemMoveCursor.mp3", 10.0f);
		RegisterSEClip("SESystemOpen",       L"resources/game/td_3105/Audio/SE/SystemOpen.mp3",       10.0f);
	}

	// -------- 登録API --------
	static void RegisterBGMClip(const std::string& name, const std::wstring& filePath, float clipVolume = 1.0f) {
		No::SoundLoad(filePath, name);
		RegisterBGM(name, clipVolume);
		No::SetVolume(name, GetEffectiveBGMVolume(name));
	}

	static void RegisterBGMClip(const std::string& name, const std::string& filePath, float clipVolume = 1.0f) {
		RegisterBGMClip(name, NoEngine::ConvertString(filePath), clipVolume);
	}

	static void RegisterSEClip(const std::string& name, const std::wstring& filePath, float clipVolume = 1.0f) {
		No::SoundLoad(filePath, name);
		RegisterSE(name, clipVolume);
	}

	static void RegisterSEClip(const std::string& name, const std::string& filePath, float clipVolume = 1.0f) {
		RegisterSEClip(name, NoEngine::ConvertString(filePath), clipVolume);
	}

	// -------- Option反映 --------
	static void ApplyOptionVolumes(float master, float bgm, float se) {
		MasterVolume() = Clamp01(master);
		BGMVolume() = Clamp01(bgm);
		SEVolume() = Clamp01(se);
		RefreshAllBGMVolumes();
	}

	static void RefreshAllBGMVolumes() {
		for (const auto& [name, clipVolume] : RegisteredBGMVolumes()) {
			No::SetVolume(name, GetEffectiveBGMVolume(name));
		}
	}

	// -------- 再生API --------
	static void PlayBGMClip(const std::string& name, bool loop = true) {
		EnsureBGMRegistered(name);
		No::SoundPlay(name, GetEffectiveBGMVolume(name), loop);
	}

	static void StopBGMClip(const std::string& name) {
		No::SoundStop(name);
	}

	static void PlaySEClip(const std::string& name) {
		EnsureSERegistered(name);
		No::SoundEffectPlay(name, GetEffectiveSEVolume(name));
	}

	// 既存互換API
	static void PlayBGM(const std::string& name, bool loop) {
		PlayBGMClip(name, loop);
	}

	static void StopBGM(const std::string& name) {
		StopBGMClip(name);
	}

	static void PlaySE(const std::string& name) {
		PlaySEClip(name);
	}

	static void PlayTestBGM(bool loop = true) {
		PlayBGMClip(TestBGMName(), loop);
	}

	static void StopTestBGM() {
		StopBGMClip(TestBGMName());
	}

	static void PlayTestSE() {
		PlaySEClip(TestSEName());
	}

	// -------- 参照 --------
	static float GetMasterVolume() { return MasterVolume(); }
	static float GetBGMVolume() { return BGMVolume(); }
	static float GetSEVolume() { return SEVolume(); }
	static float GetEffectiveBGMVolume() { return MasterVolume() * BGMVolume(); }
	static float GetEffectiveSEVolume() { return MasterVolume() * SEVolume(); }

	static float GetEffectiveBGMVolume(const std::string& name) {
		return GetEffectiveBGMVolume() * GetBGMClipVolume(name);
	}

	static float GetEffectiveSEVolume(const std::string& name) {
		return GetEffectiveSEVolume() * GetSEClipVolume(name);
	}

private:
	static float& MasterVolume() {
		static float s = 1.0f;
		return s;
	}
	static float& BGMVolume() {
		static float s = 1.0f;
		return s;
	}
	static float& SEVolume() {
		static float s = 1.0f;
		return s;
	}

	static std::unordered_map<std::string, float>& RegisteredBGMVolumes() {
		static std::unordered_map<std::string, float> s;
		return s;
	}

	static std::unordered_map<std::string, float>& RegisteredSEVolumes() {
		static std::unordered_map<std::string, float> s;
		return s;
	}

	static const std::string& TestBGMName() {
		static const std::string kName = "cb_test_bgm";
		return kName;
	}
	static const std::string& TestSEName() {
		static const std::string kName = "cb_test_se";
		return kName;
	}

	static void RegisterBGM(const std::string& name, float clipVolume) {
		RegisteredBGMVolumes()[name] = Clamp01(clipVolume);
	}

	static void RegisterSE(const std::string& name, float clipVolume) {
		RegisteredSEVolumes()[name] = Clamp01(clipVolume);
	}

	static void EnsureBGMRegistered(const std::string& name) {
		if (RegisteredBGMVolumes().find(name) == RegisteredBGMVolumes().end()) {
			RegisterBGM(name, 1.0f);
		}
	}

	static void EnsureSERegistered(const std::string& name) {
		if (RegisteredSEVolumes().find(name) == RegisteredSEVolumes().end()) {
			RegisterSE(name, 1.0f);
		}
	}

	static float GetBGMClipVolume(const std::string& name) {
		auto& map = RegisteredBGMVolumes();
		auto it = map.find(name);
		if (it == map.end()) {
			return 1.0f;
		}
		return it->second;
	}

	static float GetSEClipVolume(const std::string& name) {
		auto& map = RegisteredSEVolumes();
		auto it = map.find(name);
		if (it == map.end()) {
			return 1.0f;
		}
		return it->second;
	}

	static float Clamp01(float v) {
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}
};
}
