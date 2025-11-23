#include "CommandAllocator.h"

namespace NoEngine {
CommandAllocator::CommandAllocator(ID3D12Device* device) {
	// コマンドアロケータを生成します。
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成が上手くいかなかった場合はassertで止めます。
	assert(SUCCEEDED(hr));
}

CommandAllocator::~CommandAllocator() {}
}