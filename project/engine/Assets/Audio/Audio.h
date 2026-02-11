#pragma once

namespace NoEngine {
void AudioInitialize();
void AudioShutdown();

namespace Asset {
/// <summary>
/// 音声データ読み込み
/// </summary>
/// <param name="filePath">ファイルパス</param>
/// <param name="name">音声データ識別用の名前</param>
void SoundLoad(std::wstring filePath, std::string name);

/// <summary>
/// 音声データアンロード
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
void SoundUnload(std::string name);

/// <summary>
/// 音声データ再生(ループ再生する場合は停止する関数を呼ぶまでループし続ける)
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
/// <param name="volume">音量</param>
/// <param name="isLoop">ループ再生するかどうか</param>
void SoundPlay(std::string name, float volume, bool isLoop);

/// <summary>
/// 音声データを重なりを許して再生
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
/// <param name="volume">音量</param>
void SoundEffectPlay(std::string name, float volume);

/// <summary>
/// 音声データ再生一時停止
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
void SoundStop(std::string name);

/// <summary>
/// 音声データ再生完全停止(次回再生するときは最初から)
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
void SoundCompleteStop(std::string name);

/// <summary>
/// 音量のセッタ
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
/// <param name="volume">音量</param>
void SetVolume(std::string name, float volume);

/// <summary>
/// 音楽のピッチ調節
/// </summary>
/// <param name="name">音声データ識別用の名前</param>
/// <param name="pitch">ピッチ</param>
void SetPitch(std::string name, float pitch);
}
}