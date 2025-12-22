#pragma once
#include "../GpuResource.h"

namespace NoEngine {

enum {
    kGpuAllocatorPageSize = 0x10000,	// 64K
    kCpuAllocatorPageSize = 0x200000	// 2MB
};

/// <summary>
/// ページ単位のメモリブロック
/// </summary>
class LinearAllocationPage : public GpuResource {

public:
    LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES usage);

    ~LinearAllocationPage();

    void Map(void);

    void Unmap(void);

    void* cpuVirtualAddress_;
    D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress_;
};
}

