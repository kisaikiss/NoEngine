#include "DescriptorAllocator.h"

#include "../GraphicsCore.h"

namespace NoEngine {
std::mutex DescriptorAllocator::sAllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sDescriptorHeapPool;

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t count) {
	if (currentHeap_ == nullptr || remainingFreeHandles_ < count) {
		currentHeap_ = RequestNewHeap(type_);
		currentHandle_ = currentHeap_->GetCPUDescriptorHandleForHeapStart();
		remainingFreeHandles_ = sNumDescriptorsPerHeap;

		if (descriptorSize_ == 0)
			descriptorSize_ = GraphicsCore::gGraphicsDevice->GetDevice()->GetDescriptorHandleIncrementSize(type_);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = currentHandle_;
	currentHandle_.ptr += count * descriptorSize_;
	remainingFreeHandles_ -= count;
	return ret;
}

void DescriptorAllocator::DestroyAll(void) {
	sDescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	std::lock_guard<std::mutex> LockGuard(sAllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Type = type;
	desc.NumDescriptors = sNumDescriptorsPerHeap;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
	HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
	if (FAILED(hr)) {
		assert(false);
	}
	sDescriptorHeapPool.emplace_back(heap);
	return heap.Get();
}

}