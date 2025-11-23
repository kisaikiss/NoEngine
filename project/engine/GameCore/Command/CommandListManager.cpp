#include "CommandListManager.h"

namespace NoEngine {
CommandListManager::CommandListManager() :
	device_(nullptr),
	graphicsQueue_(D3D12_COMMAND_LIST_TYPE_DIRECT),
	computeQueue_(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	copyQueue_(D3D12_COMMAND_LIST_TYPE_COPY)
{}

CommandListManager::~CommandListManager() {
	Shutdown();
}

void CommandListManager::Create(ID3D12Device* device) {
	assert(device != nullptr);

	device_ = device;

	graphicsQueue_.Create(device);
	computeQueue_.Create(device);
	copyQueue_.Create(device);
}

void CommandListManager::Shutdown() {
	graphicsQueue_.Shutdown();
	computeQueue_.Shutdown();
	copyQueue_.Shutdown();
}
void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>* list, ID3D12CommandAllocator** allocator) {
	assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE);
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT: *allocator = graphicsQueue_.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: *allocator = computeQueue_.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_COPY: *allocator = copyQueue_.RequestAllocator(); break;
	}

	HRESULT hr =device_->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(&list));
	assert(SUCCEEDED(hr));
	(*list)->SetName(L"CommandList");
}
}