#include "CommandListManager.h"

#include "engine/Debug/Logger/Log.h"
#include "../GraphicsCore.h"

namespace NoEngine {
CommandListManager::CommandListManager() :
	graphicsQueue_(D3D12_COMMAND_LIST_TYPE_DIRECT),
	computeQueue_(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	copyQueue_(D3D12_COMMAND_LIST_TYPE_COPY)
{}

CommandListManager::~CommandListManager() {
	Shutdown();
}

void CommandListManager::Create() {
	Log::DebugPrint("CommandListManager Create Start");

	graphicsQueue_.Create();
	computeQueue_.Create();
	copyQueue_.Create();
	Log::DebugPrint("CommandListManager Created!");
}

void CommandListManager::Shutdown() {
	graphicsQueue_.Shutdown();
	computeQueue_.Shutdown();
	copyQueue_.Shutdown();
}
void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList4** list, ID3D12CommandAllocator** allocator) {
	assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE);
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT: *allocator = graphicsQueue_.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: *allocator = computeQueue_.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_COPY: *allocator = copyQueue_.RequestAllocator(); break;
	}

	HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(list));
	if (FAILED(hr)) {
		assert(false);
	}
	(*list)->SetName(L"CommandList");
}

void CommandListManager::WaitForFence(uint64_t FenceValue) {
	CommandQueue& producer = GraphicsCore::gCommandListManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	producer.WaitForFence(FenceValue);
}

}