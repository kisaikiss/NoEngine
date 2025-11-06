#include "GraphicsDevice.h"
#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
namespace Graphics {
GraphicsDevice::GraphicsDevice(IDXGIAdapter4* adapter) {
	Log::DebugPrint("GraphicsDevice create start");
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを生成
		HRESULT hr = D3D12CreateDevice(adapter, featureLevels[i], IID_PPV_ARGS(&device_));
		// 選定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたときにログ出力を行ってループを抜ける
			Log::DebugPrint(std::format("FeatureLevel : {}", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Log::DebugPrint("Complete create D3D12Device!!!");// 初期化完了のログをだす
}
}
}