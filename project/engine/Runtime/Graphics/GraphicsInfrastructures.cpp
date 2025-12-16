#include "GraphicsInfrastructures.h"

#include "engine/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

NoEngine::Graphics::GraphicsInfrastructures::GraphicsInfrastructures() {
	Log::DebugPrint("GraphicsInfrastructures create start");
	//DXGIファクトリーの生成
	//HRESULTはWindows系のエラーコード
	//関数が成功したかSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//初期化の根本的な部分でエラーが出た場合のためにassertする
	assert(SUCCEEDED(hr));

	// 良い順にアダプタを検討
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプタの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = dxgiAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できなかったら一大事
		// ソフトウェアアダプタは除外
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 利用したアダプタの情報をコンソールに出力。wstringのままだと注意
			Log::DebugPrint(ConvertString(std::format(L"Use Adapter: {}", adapterDesc.Description)));
			break;
		}
		dxgiAdapter_ = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}
	// 適切なアダプタが見つからなかったので起動できない
	assert(dxgiAdapter_ != nullptr);
	Log::DebugPrint("GraphicsInfrastructures created");
}
