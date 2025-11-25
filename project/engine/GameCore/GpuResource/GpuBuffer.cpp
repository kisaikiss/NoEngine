#include "GpuBuffer.h"
#include "../Command/CommandContext.h"
#include "../GraphicsCore.h"

namespace NoEngine {

void GpuBuffer::Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData) {

    Destroy();

    elementCount_ = numElements;
    elementSize_ = elementSize;
    bufferSize_ = numElements * elementSize;

    D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

    usageState_ = D3D12_RESOURCE_STATE_COMMON;

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
        &ResourceDesc, usageState_, nullptr, IID_PPV_ARGS(&resource_));
    if (FAILED(hr)) {
        assert(false);
    }
    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();

    if (initialData) {
       // CommandContext::InitializeBuffer(*this, initialData, bufferSize_);
    }


#ifdef RELEASE
    (name);
#else
    resource_->SetName(name.c_str());
#endif

    CreateDerivedViews();
}


D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer(void) {
    return D3D12_RESOURCE_DESC();
}

}
