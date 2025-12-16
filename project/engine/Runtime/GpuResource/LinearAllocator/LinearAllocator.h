#pragma once
#include "../GpuResource.h"

namespace NoEngine {
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
};
}

