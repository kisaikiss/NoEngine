#pragma once
#include "DescriptorHeap.h"
#include "../PipelineStateObject/RootSignature.h"

namespace NoEngine {
class CommandContext;

/// <summary>
/// 動的に生成されるディスクリプタテーブルのための線形割り当てシステム
/// </summary>
class DynamicDescriptorHeap {

public:
    DynamicDescriptorHeap(CommandContext& owningContext, D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    ~DynamicDescriptorHeap();

    static void DestroyAll(void) {
        sDescriptorHeapPool[0].clear();
        sDescriptorHeapPool[1].clear();
    }

    void CleanupUsedHeaps(uint64_t fenceValue);

    // 指定されたルート パラメータ用に予約されたキャッシュ領域に複数のハンドルをコピーします。
    void SetGraphicsDescriptorHandles(UINT rootIndex, UINT offset, UINT numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[]) {
        graphicsHandleCache_.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
    }

    void SetComputeDescriptorHandles(UINT rootIndex, UINT offset, UINT numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE handles[]) {
        computeHandleCache_.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
    }

    // キャッシュをバイパスし、シェーダーが認識できるヒープに直接アップロードする
    D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE handles);

    // ルート署名に必要な記述子テーブルをサポートするために必要なキャッシュ レイアウトを推測します。
    void ParseGraphicsRootSignature(const RootSignature& rootSignature) {
        graphicsHandleCache_.ParseRootSignature(descriptorType_, rootSignature);
    }

    void ParseComputeRootSignature(const RootSignature& rootSignature) {
        computeHandleCache_.ParseRootSignature(descriptorType_, rootSignature);
    }

    // キャッシュ内の新しい記述子をシェーダーが認識できるヒープにアップロードします。
    inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList4* commandList) {
        if (graphicsHandleCache_.staleRootParamsBitMap_ != 0)
            CopyAndBindStagedTables(graphicsHandleCache_, commandList, &ID3D12GraphicsCommandList4::SetGraphicsRootDescriptorTable);
    }

    inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList4* commandList) {
        if (computeHandleCache_.staleRootParamsBitMap_ != 0)
            CopyAndBindStagedTables(computeHandleCache_, commandList, &ID3D12GraphicsCommandList4::SetComputeRootDescriptorTable);
    }

private:

    // Static members
    static const uint32_t kNumDescriptorsPerHeap = 1024;
    static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sDescriptorHeapPool[2];
    // Static methods
    static ID3D12DescriptorHeap* RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    static void DiscardDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint64_t fenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& usedHeaps);

    // Non-static members
    CommandContext& owningContext_;
    ID3D12DescriptorHeap* currentHeapPtr_;
    const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType_;
    uint32_t descriptorSize_;
    uint32_t currentOffset_;
    DescriptorHandle firstDescriptor_;
    std::vector<ID3D12DescriptorHeap*> retiredHeaps_;

    // Describes a descriptor table entry:  a region of the handle cache and which handles have been set
    struct DescriptorTableCache {
        DescriptorTableCache() : AssignedHandlesBitMap(0) {}
        uint32_t AssignedHandlesBitMap;
        D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
        uint32_t TableSize;
    };

    struct DescriptorHandleCache {
        DescriptorHandleCache() {
            ClearCache();
        }

        void ClearCache() {
            rootDescriptorTablesBitMap_ = 0;
            staleRootParamsBitMap_ = 0;
            maxCachedDescriptors_ = 0;
        }

        uint32_t rootDescriptorTablesBitMap_;
        uint32_t staleRootParamsBitMap_;
        uint32_t maxCachedDescriptors_;

        static const uint32_t kMaxNumDescriptors = 256;
        static const uint32_t kMaxNumDescriptorTables = 16;

        uint32_t ComputeStagedSize();
        void CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize, DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList4* CmdList,
            void (STDMETHODCALLTYPE ID3D12GraphicsCommandList4::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

        DescriptorTableCache rootDescriptorTable_[kMaxNumDescriptorTables];
        D3D12_CPU_DESCRIPTOR_HANDLE handleCache_[kMaxNumDescriptors];

        void UnbindAllValid();
        void StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
        void ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE type, const RootSignature& rootSig);
    };

    DescriptorHandleCache graphicsHandleCache_;
    DescriptorHandleCache computeHandleCache_;

    bool HasSpace(uint32_t Count) {
        return (currentHeapPtr_ != nullptr && currentOffset_ + Count <= kNumDescriptorsPerHeap);
    }

    void RetireCurrentHeap(void);
    void RetireUsedHeaps(uint64_t fenceValue);
    ID3D12DescriptorHeap* GetHeapPointer();

    DescriptorHandle Allocate(UINT Count) {
        DescriptorHandle ret = firstDescriptor_ + currentOffset_ * descriptorSize_;
        currentOffset_ += Count;
        return ret;
    }

    void CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList4* CmdList,
        void (STDMETHODCALLTYPE ID3D12GraphicsCommandList4::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

    // Mark all descriptors in the cache as stale and in need of re-uploading.
    void UnbindAllValid(void);
};
}

