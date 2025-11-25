#pragma once
#include "../GpuResource.h"

namespace NoEngine {
/// <summary>
/// ページ単位のメモリブロック
/// </summary>
class LinearAllocationPage : public GpuResource {

public:
    LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage);

    ~LinearAllocationPage();

    void Map(void);

    void Unmap(void);

    void* cpuVirtualAddress_;
    D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress_;
};
}

