#pragma once
#include "../GpuResource.h"
#include "LinearAllocationPage.h"
#include "LinearAllocatorType.h"
#include "LinearAllocatorPageManager.h"

namespace NoEngine {

// 定数ブロックは、それぞれ 16 バイトの 16 個の定数の倍数である必要があります。
#define DEFAULT_ALIGN 256

/// <summary>
/// 動的割り当て
/// </summary>
struct DynAlloc {
    DynAlloc(GpuResource& BaseResource, size_t ThisOffset, size_t ThisSize)
        : Buffer(BaseResource), Offset(ThisOffset), Size(ThisSize) {
    }

    GpuResource& Buffer;	// このメモリに関連付けられたD3D12Buffer。
    size_t Offset;			// バッファリソースの開始からのオフセット
    size_t Size;			// この割り当ての予約サイズ
    void* DataPtr;			// CPU書き込み可能アドレス
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// GPUで見えるアドレス
};


class LinearAllocator {

public:

    LinearAllocator(LinearAllocatorType type) : allocationType_(type), pageSize_(0), curOffset_(~(size_t)0), curPage_(nullptr) {
        assert(type > LinearAllocatorType::kInvalidAllocator && type < LinearAllocatorType::kNumAllocatorTypes);
        pageSize_ = (type == LinearAllocatorType::kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
    }

    DynAlloc Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

    void CleanupUsedPages(uint64_t FenceID);

    static void DestroyAll(void) {
        sPageManager_[0].Destroy();
        sPageManager_[1].Destroy();
    }

private:

    DynAlloc AllocateLargePage(size_t SizeInBytes);

    static LinearAllocatorPageManager sPageManager_[2];

    LinearAllocatorType allocationType_;
    size_t pageSize_;
    size_t curOffset_;
    LinearAllocationPage* curPage_;
    std::vector<LinearAllocationPage*> retiredPages_;
    std::vector<LinearAllocationPage*> largePageList_;
};
}

