#include "DescriptorHeap.h"
#include "../GraphicsCore.h"

#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
void DescriptorHeap::Create(const std::wstring& debugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount) {
    heapDesc_.Type = type;
    heapDesc_.NumDescriptors = maxCount;
    heapDesc_.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc_.NodeMask = 1;

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateDescriptorHeap(&heapDesc_, IID_PPV_ARGS(heap_.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(hr));

#ifdef RELEASE_BUILD
    (void)debugHeapName;
#else
    heap_->SetName(debugHeapName.c_str());
#endif

    descriptorSize_ = GraphicsCore::gGraphicsDevice->GetDevice()->GetDescriptorHandleIncrementSize(heapDesc_.Type);
    numFreeDescriptors_ = heapDesc_.NumDescriptors;
    firstHandle_ = DescriptorHandle(
        heap_->GetCPUDescriptorHandleForHeapStart(),
        heap_->GetGPUDescriptorHandleForHeapStart());
    nextFreeHandle_ = firstHandle_;
}

DescriptorHandle DescriptorHeap::Alloc(uint32_t count) {
    if (!HasAvailableSpace(count)) {
        Log::DebugPrint("Descriptor Heap out of space.  Increase heap size.", VerbosityLevel::kCritical);
        assert(false);
    }
    DescriptorHandle ret = nextFreeHandle_;
    nextFreeHandle_ += count * descriptorSize_;
    numFreeDescriptors_ -= count;
    return ret;
}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& descriptorHandle) const {
    if (descriptorHandle.GetCpuPtr() < firstHandle_.GetCpuPtr() ||
        descriptorHandle.GetCpuPtr() >= firstHandle_.GetCpuPtr() + heapDesc_.NumDescriptors * descriptorSize_) {
        return false;
    }

    if (descriptorHandle.GetGpuPtr() - firstHandle_.GetGpuPtr() !=
        descriptorHandle.GetCpuPtr() - firstHandle_.GetCpuPtr()) {
        return false;
    }

    return true;
}

}