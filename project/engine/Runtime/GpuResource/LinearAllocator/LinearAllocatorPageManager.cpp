#include "LinearAllocatorPageManager.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/Command/CommandListManager.h"

namespace NoEngine {
LinearAllocatorType LinearAllocatorPageManager::sAutoType = LinearAllocatorType::kGpuExclusive;

LinearAllocatorPageManager::LinearAllocatorPageManager() {
    allocationType_ = sAutoType;
    sAutoType = static_cast<LinearAllocatorType>(static_cast<uint32_t>(sAutoType) + 1);
    assert(sAutoType <= LinearAllocatorType::kNumAllocatorTypes);
}

LinearAllocationPage* LinearAllocatorPageManager::RequestPage() {
    std::lock_guard<std::mutex> LockGuard(mutex_);

    while (!retiredPages_.empty() && GraphicsCore::gCommandListManager.IsFenceComplete(retiredPages_.front().first)) {
        availablePages_.push(retiredPages_.front().second);
        retiredPages_.pop();
    }

    LinearAllocationPage* pagePtr = nullptr;

    if (!availablePages_.empty()) {
        pagePtr = availablePages_.front();
        availablePages_.pop();
    } else {
        pagePtr = CreateNewPage();
        pagePool_.emplace_back(pagePtr);
    }

    return pagePtr;
}

LinearAllocationPage* LinearAllocatorPageManager::CreateNewPage(size_t pageSize) {
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_RESOURCE_STATES defaultUsage;

    if (allocationType_ == LinearAllocatorType::kGpuExclusive) {
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        resourceDesc.Width = pageSize == 0 ? kGpuAllocatorPageSize : pageSize;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        defaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    } else {
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        resourceDesc.Width = pageSize == 0 ? kCpuAllocatorPageSize : pageSize;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        defaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    ID3D12Resource* pBuffer;
    HRESULT hr = (GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, defaultUsage, nullptr, IID_PPV_ARGS(&pBuffer)));

    if (FAILED(hr)) {
        assert(false);
    }

    pBuffer->SetName(L"LinearAllocator Page");

    return new LinearAllocationPage(pBuffer, defaultUsage);
}

void LinearAllocatorPageManager::DiscardPages(uint64_t fenceValue, const std::vector<LinearAllocationPage*>& usedPages) {
    std::lock_guard<std::mutex> LockGuard(mutex_);
    for (auto iter = usedPages.begin(); iter != usedPages.end(); ++iter)
       retiredPages_.push(std::make_pair(fenceValue, *iter));
}

void LinearAllocatorPageManager::FreeLargePages(uint64_t fenceValue, const std::vector<LinearAllocationPage*>& largePages) {
    std::lock_guard<std::mutex> LockGuard(mutex_);

    while (!deletionQueue_.empty() && GraphicsCore::gCommandListManager.IsFenceComplete(deletionQueue_.front().first)) {
        delete deletionQueue_.front().second;
        deletionQueue_.pop();
    }

    for (auto iter = largePages.begin(); iter != largePages.end(); ++iter) {
        (*iter)->Unmap();
        deletionQueue_.push(std::make_pair(fenceValue, *iter));
    }
}

}
