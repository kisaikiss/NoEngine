#include "GraphicsCore.h"

#include "Command/CommandListManager.h"
#include "ContextManager.h"

#include "../Debug/Logger/Log.h"

namespace NoEngine {
using namespace std;

namespace GraphicsCore {
std::unique_ptr<Graphics::GraphicsInfrastructures> gGraphicsInfrastructures;
std::unique_ptr<Graphics::GraphicsDevice> gGraphicsDevice;
CommandListManager gCommandListManager;
ContextManager gContextManager;

DescriptorAllocator gDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

void Initialize() {
	EnableDebugLayer();
	gGraphicsInfrastructures = make_unique<Graphics::GraphicsInfrastructures>();
	gGraphicsDevice = make_unique<Graphics::GraphicsDevice>(gGraphicsInfrastructures->GetDXGIAdapter());
	gCommandListManager.Create();
	SettingDebugLayer();
}

void Shutdown(void) {
	for (auto& descriptorAllocator : gDescriptorAllocator) {
		descriptorAllocator.DestroyAll();
	}


	gCommandListManager.Shutdown();

	gGraphicsDevice.reset();
	gGraphicsInfrastructures.reset();
}

void EnableDebugLayer() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
}

void SettingDebugLayer() {
#ifdef _DEBUG
	if (!gGraphicsDevice) {
		Log::DebugPrint("GraphicsDevice is nulptr!!!", VerbosityLevel::kCritical);
		assert(false);
	}
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(gGraphicsDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッガーレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		// 謎の警告を一旦無視するようにする。
		D3D12_MESSAGE_ID hide[] = { D3D12_MESSAGE_ID_FENCE_ZERO_WAIT };
		D3D12_INFO_QUEUE_FILTER filterFenceZero = {};
		filterFenceZero.DenyList.NumIDs = _countof(hide);
		filterFenceZero.DenyList.pIDList = hide;
		infoQueue->AddStorageFilterEntries(&filterFenceZero);

		// 解放
		infoQueue->Release();
	}
#endif
}
}
}