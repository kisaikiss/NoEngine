#include "CommandAllocator.h"

namespace NoEngine {
CommandAllocator::CommandAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device) {
	// コマンドアロケータを生成します。
	HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成が上手くいかなかった場合はassertで止めます。
	if (FAILED(hr)) {
		assert(false);
	}
}

CommandAllocator::~CommandAllocator() {}
}