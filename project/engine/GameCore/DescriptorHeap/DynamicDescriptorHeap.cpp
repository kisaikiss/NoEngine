#include "DynamicDescriptorHeap.h"
#include "../GraphicsCore.h"

namespace {
std::mutex sMutex;
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sRetiredDescriptorHeaps[2];
std::queue<ID3D12DescriptorHeap*> sAvailableDescriptorHeaps[2];
}

namespace NoEngine {
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::sDescriptorHeapPool[2];

DynamicDescriptorHeap::DynamicDescriptorHeap(CommandContext& owningContext, D3D12_DESCRIPTOR_HEAP_TYPE heapType) :
	owningContext_(owningContext),
	descriptorType_(heapType) {
	currentHeapPtr_ = nullptr;
	currentOffset_ = 0;
	descriptorSize_ = GraphicsCore::gGraphicsDevice->GetDevice()->GetDescriptorHandleIncrementSize(heapType);
}

DynamicDescriptorHeap::~DynamicDescriptorHeap() {}

void DynamicDescriptorHeap::CleanupUsedHeaps(uint64_t fenceValue) {
	//RetireCurrentHeap();
	//RetireUsedHeaps(fenceValue);
	(void)fenceValue;
	graphicsHandleCache_.ClearCache();
	computeHandleCache_.ClearCache();
}

}