#pragma once
namespace NoEngine {

/// <summary>
/// 無制限のリソース記述子(ディスクリプタ)アロケータ。リソースの作成時にCPUから参照可能なリソース記述子のための領域を提供することを目的としています。
/// </summary>
class DescriptorAllocator {

public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) :
        type_(Type), currentHeap_(nullptr), descriptorSize_(0) {
        currentHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    /// <summary>
    /// 指定数のディスクリプタを確保します。
    /// </summary>
    /// <param name="Count">確保するディスクリプタの数</param>
    /// <returns>ディスクリプタハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);

    /// <summary>
    /// 全てのディスクリプタヒープを破棄します。
    /// </summary>
    static void DestroyAll(void);

protected:

    static inline const uint32_t sNumDescriptorsPerHeap = 256;
    static std::mutex sAllocationMutex;
    static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sDescriptorHeapPool;

    /// <summary>
    /// 新しいディスクリプタヒープをリクエストします。
    /// </summary>
    /// <param name="type">ヒープのタイプ</param>
    /// <returns>ディスクリプタヒープのポインタ</returns>
    static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

    D3D12_DESCRIPTOR_HEAP_TYPE type_;
    ID3D12DescriptorHeap* currentHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE currentHandle_;
    uint32_t descriptorSize_;
    uint32_t remainingFreeHandles_;
};


}