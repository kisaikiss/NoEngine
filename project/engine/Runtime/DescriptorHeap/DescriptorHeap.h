#pragma once
#include "DescriptorHandle.h"

namespace NoEngine {

class DescriptorHeap {

public:

    DescriptorHeap(void) {}
    ~DescriptorHeap(void) { Destroy(); }

    /// <summary>
    /// ディスクリプタヒープを作成します。
    /// </summary>
    /// <param name="debugHeapName">デバッグ用の名前</param>
    /// <param name="type">ディスクリプタヒープタイプ</param>
    /// <param name="maxCount">ディスクリプタの数</param>
    void Create(const std::wstring& debugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount);

    /// <summary>
    /// ディスクリプタヒープを破棄します。
    /// </summary>
    void Destroy(void) { heap_ = nullptr; }

    /// <summary>
    /// 指定した数ヒープにスペースが開いているか判別します。
    /// </summary>
    /// <param name="count">スペースが空いているかどうか(true : 空いている、false : 空いていない)</param>
    /// <returns></returns>
    bool HasAvailableSpace(uint32_t count) const { return count <= numFreeDescriptors_; }

    /// <summary>
    /// ディスクリプタハンドルを指定した数だけ割り当てます。
    /// </summary>
    /// <param name="count">割り当て数</param>
    /// <returns>割り当てたディスクリプタハンドル</returns>
    DescriptorHandle Alloc(uint32_t count = 1);

    DescriptorHandle operator[] (uint32_t arrayIdx) const { return firstHandle_ + arrayIdx * descriptorSize_; }


    uint32_t GetOffsetOfHandle(const DescriptorHandle& descriptorHandle) {
        return (uint32_t)(descriptorHandle.GetCpuPtr() - firstHandle_.GetCpuPtr()) / descriptorSize_;
    }

    /// <summary>
    /// 引数で指定したディスクリプタハンドルがこのヒープで利用可能かどうか検証します。
    /// </summary>
    /// <param name="descriptorHandle">ディスクリプタハンドル</param>
    /// <returns>利用可能かどうか(true : 可能、false : 不可)</returns>
    bool ValidateHandle(const DescriptorHandle& descriptorHandle) const;

    ID3D12DescriptorHeap* GetHeapPointer() const { return heap_.Get(); }

    uint32_t GetDescriptorSize(void) const { return descriptorSize_; }

private:

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc_;
    uint32_t descriptorSize_;
    uint32_t numFreeDescriptors_;
    DescriptorHandle firstHandle_;
    DescriptorHandle nextFreeHandle_;
};


}