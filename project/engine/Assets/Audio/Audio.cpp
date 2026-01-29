#include "Audio.h"

#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"

#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace NoEngine {

namespace {

struct SoundData {
	WAVEFORMATEX* waveFormat = nullptr; // WAVEフォーマット
	BYTE* pBuffer = nullptr; // 音声データ
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	std::vector<BYTE> mediaData;
	std::string name; // 名
	std::wstring filePath;
};

Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
IXAudio2MasteringVoice* masterVoice = nullptr;
std::map<std::string, SoundData> soundData;
}

/// <summary>
/// 音声データアンロード
/// </summary>
/// <param name="soundDataPointer">音声データのポインタ</param>
static void SoundUnload(SoundData* soundDataPointer) {
	//バッファのメモリを解放
	delete[] soundDataPointer->pBuffer;
	soundDataPointer->pBuffer = 0;
	soundDataPointer->mediaData.clear();
	CoTaskMemFree(soundDataPointer->waveFormat);
	Log::DebugPrint("sound data unloaded. name : " + soundDataPointer->name);
}

void AudioInitialize() {
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
}

void AudioShutdown() {
	xAudio2.Reset();
	for (auto& [key, sound] : soundData) {
		SoundUnload(&sound);
	}
	MFShutdown();
	Log::DebugPrint("Audio Finalized\n",VerbosityLevel::kInfo);
}

namespace Audio {

void SoundLoad(std::wstring filePath, std::string name) {
	Log::DebugPrint("Sound lord begin. filePath : " + ConvertString(filePath) + ", name : " + name + "\n");

	//まだ読み込んでいなければ
	if (soundData[name].filePath != filePath) {
		if (soundData[name].name != name) {
			soundData[name].name = name;
		} else {
			Log::DebugPrint("SoundData_name : " + name + " is already taken\n");
			assert(false);
			return;
		}

		soundData[name].filePath = filePath;

	} else {
		Log::DebugPrint("filePath : " + ConvertString(filePath) + " is already loaded\n");
		return;
	}
	//ソースリーダの作成
	IMFSourceReader* pMFSourceReader = nullptr;
	HRESULT hr = MFCreateSourceReaderFromURL(filePath.c_str(), NULL, &pMFSourceReader);
	assert(SUCCEEDED(hr));
	//メディアタイプの取得
	IMFMediaType* pMFMediaType = nullptr;
	MFCreateMediaType(&pMFMediaType);
	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pMFSourceReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType);
	assert(SUCCEEDED(hr));

	pMFMediaType->Release();
	pMFMediaType = nullptr;
	hr = pMFSourceReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pMFMediaType);
	assert(SUCCEEDED(hr));


	//オーディオデータ形式の作成
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &soundData[name].waveFormat, nullptr);
	assert(SUCCEEDED(hr));

	pMFMediaType->Release();

	while (true) {
		IMFSample* pMFSample{ nullptr };
		DWORD dwStreamFlags{ 0 };
		pMFSourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		IMFMediaBuffer* pMFMediaBuffer{ nullptr };
		pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		BYTE* pBuffer{ nullptr };
		DWORD cbCurrentLength{ 0 };
		pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

		soundData[name].mediaData.resize(soundData[name].mediaData.size() + cbCurrentLength);
		memcpy(soundData[name].mediaData.data() + soundData[name].mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

		pMFMediaBuffer->Unlock();

		pMFMediaBuffer->Release();
		pMFSample->Release();

	}

	pMFSourceReader->Release();

	//波形フォーマットを元にSourceVoiceの生成
	hr = xAudio2->CreateSourceVoice(&soundData[name].pSourceVoice, soundData[name].waveFormat);
	assert(SUCCEEDED(hr));
	Log::DebugPrint("Sound lord Succes! filePath : " + ConvertString(filePath) + ", name : " + name + "\n");
}

void SoundUnload(std::string name) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	//バッファのメモリを解放
	delete[] soundData[name].pBuffer;
	soundData[name].pBuffer = 0;
	soundData[name].mediaData.clear();
	soundData[name].waveFormat = {};
	Log::DebugPrint("sound data unloaded. name : " + name + "\n");
}

void SoundPlay(std::string name, float volume, bool isLoop) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	HRESULT result;

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	if (isLoop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	buf.pAudioData = soundData[name].mediaData.data();
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(soundData[name].mediaData.size());

	//波形データの再生
	result = soundData[name].pSourceVoice->SubmitSourceBuffer(&buf);
	result = soundData[name].pSourceVoice->Start();
	assert(SUCCEEDED(result));
	soundData[name].pSourceVoice->SetVolume(volume);
}

void SoundEffectPlay(std::string name, float volume) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	IXAudio2SourceVoice* pSourceVoice;
	HRESULT hr = xAudio2->CreateSourceVoice(&pSourceVoice, soundData[name].waveFormat);

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData[name].mediaData.data();
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(soundData[name].mediaData.size());

	//波形データの再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	hr = pSourceVoice->Start();
	assert(SUCCEEDED(hr));
	pSourceVoice->SetVolume(volume);
}

void SoundStop(std::string name) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	HRESULT hr = soundData[name].pSourceVoice->Stop();
	(void)hr;
	assert(SUCCEEDED(hr));
}

void SoundCompleteStop(std::string name) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	HRESULT hr = soundData[name].pSourceVoice->Stop();
	assert(SUCCEEDED(hr));
	hr = soundData[name].pSourceVoice->FlushSourceBuffers();
	assert(SUCCEEDED(hr));
}

void SetVolume(std::string name, float volume) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	soundData[name].pSourceVoice->SetVolume(volume);
}

void SetPitch(std::string name, float pitch) {
	if (soundData[name].name != name) {
		Log::DebugPrint("This sound name is Not Found. name : " + name + "\n");
		return;
	}
	soundData[name].pSourceVoice->SetFrequencyRatio(pitch);
}


}


}