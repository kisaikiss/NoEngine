#include "DynamicDescriptorHeap.h"
#include "../GraphicsCore.h"
#include "../Command/CommandContext.h"

namespace {
std::mutex sMutex;
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sRetiredDescriptorHeaps[2];
std::queue<ID3D12DescriptorHeap*> sAvailableDescriptorHeaps[2];
}

namespace NoEngine {
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::sDescriptorHeapPool[2];

DynamicDescriptorHeap::DynamicDescriptorHeap(CommandContext& owningContext, D3D12_DESCRIPTOR_HEAP_TYPE heapType) :
	owningContext_(owningContext),
	descriptorType_(heapType) {
	currentHeapPtr_ = nullptr;
	currentOffset_ = 0;
	descriptorSize_ = GraphicsCore::gGraphicsDevice->GetDevice()->GetDescriptorHandleIncrementSize(heapType);
}

DynamicDescriptorHeap::~DynamicDescriptorHeap() {}

void DynamicDescriptorHeap::CleanupUsedHeaps(uint64_t fenceValue) {
	RetireCurrentHeap();
	RetireUsedHeaps(fenceValue);
	graphicsHandleCache_.ClearCache();
	computeHandleCache_.ClearCache();
}

D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE handles) {
	if (!HasSpace(1)) {
		RetireCurrentHeap();
		UnbindAllValid();
	}

	owningContext_.SetDescriptorHeap(descriptorType_, GetHeapPointer());

	DescriptorHandle destHandle = firstDescriptor_ + currentOffset_ * descriptorSize_;
	currentOffset_ += 1;

	GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1, destHandle, handles, descriptorType_);

	return destHandle;
}

ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
    std::lock_guard<std::mutex> LockGuard(sMutex);

    uint32_t idx = heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;

    while (!sRetiredDescriptorHeaps[idx].empty() && GraphicsCore::gCommandListManager.IsFenceComplete(sRetiredDescriptorHeaps[idx].front().first)) {
        sAvailableDescriptorHeaps[idx].push(sRetiredDescriptorHeaps[idx].front().second);
        sRetiredDescriptorHeaps[idx].pop();
    }

    if (!sAvailableDescriptorHeaps[idx].empty()) {
        ID3D12DescriptorHeap* heapPtr = sAvailableDescriptorHeaps[idx].front();
        sAvailableDescriptorHeaps[idx].pop();
        return heapPtr;
    } else {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = heapType;
        heapDesc.NumDescriptors = kNumDescriptorsPerHeap;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask = 1;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heapPtr;
        HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapPtr));
        if (FAILED(hr)) {
            assert(false);
        }

        sDescriptorHeapPool[idx].emplace_back(heapPtr);
        return heapPtr.Get();
    }
}

void DynamicDescriptorHeap::DiscardDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint64_t fenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& usedHeaps) {
    uint32_t idx = heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;
    std::lock_guard<std::mutex> LockGuard(sMutex);
    for (auto iter = usedHeaps.begin(); iter != usedHeaps.end(); ++iter)
        sRetiredDescriptorHeaps[idx].push(std::make_pair(fenceValueForReset, *iter));
}

void DynamicDescriptorHeap::RetireCurrentHeap(void) {
    // 未使用のヒープを廃止しないでください。
    if (currentOffset_ == 0) {
        assert(currentHeapPtr_ == nullptr);
        return;
    }

    assert(currentHeapPtr_ != nullptr);
    retiredHeaps_.push_back(currentHeapPtr_);
    currentHeapPtr_ = nullptr;
    currentOffset_ = 0;
}

void DynamicDescriptorHeap::RetireUsedHeaps(uint64_t fenceValue) {
    DiscardDescriptorHeaps(descriptorType_, fenceValue, retiredHeaps_);
    retiredHeaps_.clear();
}

ID3D12DescriptorHeap* DynamicDescriptorHeap::GetHeapPointer() {
    if (currentHeapPtr_ == nullptr) {
        assert(currentOffset_ == 0);
        currentHeapPtr_ = RequestDescriptorHeap(descriptorType_);
        firstDescriptor_ = DescriptorHandle(
            currentHeapPtr_->GetCPUDescriptorHandleForHeapStart(),
            currentHeapPtr_->GetGPUDescriptorHandleForHeapStart());
    }

    return currentHeapPtr_;
}

void DynamicDescriptorHeap::CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList4* CmdList, void(__stdcall ID3D12GraphicsCommandList4::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE)) {
    uint32_t NeededSize = HandleCache.ComputeStagedSize();
    if (!HasSpace(NeededSize)) {
        RetireCurrentHeap();
        UnbindAllValid();
        NeededSize = HandleCache.ComputeStagedSize();
    }

    // これにより、新しいヒープの作成がトリガーされる可能性があります
    owningContext_.SetDescriptorHeap(descriptorType_, GetHeapPointer());
    HandleCache.CopyAndBindStaleTables(descriptorType_, descriptorSize_, Allocate(NeededSize), CmdList, SetFunc);
}

void DynamicDescriptorHeap::UnbindAllValid() {
    graphicsHandleCache_.UnbindAllValid();
    computeHandleCache_.UnbindAllValid();
}

uint32_t DynamicDescriptorHeap::DescriptorHandleCache::ComputeStagedSize() {
    // 古いディスクリプタテーブルの最大割り当てオフセットを合計して、必要な合計スペースを決定します。
    uint32_t NeededSpace = 0;
    uint32_t RootIndex;
    uint32_t StaleParams = staleRootParamsBitMap_;
    while (_BitScanForward((unsigned long*)&RootIndex, StaleParams)) {
        StaleParams ^= (1 << RootIndex);

        uint32_t MaxSetHandle{};
        assert(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, rootDescriptorTable_[RootIndex].AssignedHandlesBitMap) &&
            "Root entry marked as stale but has no stale descriptors");

        NeededSpace += MaxSetHandle + 1;
    }
    return NeededSpace;
}

void DynamicDescriptorHeap::DescriptorHandleCache::CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorSize,
    DescriptorHandle destHandleStart, ID3D12GraphicsCommandList4* cmdList, void(__stdcall ID3D12GraphicsCommandList4::* setFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE)) {
    uint32_t staleParamCount = 0;
    uint32_t tableSize[DescriptorHandleCache::kMaxNumDescriptorTables];
    uint32_t rootIndices[DescriptorHandleCache::kMaxNumDescriptorTables];
    uint32_t neededSpace = 0;
    uint32_t rootIndex;

    // 古いディスクリプタテーブルの最大割り当てオフセットを合計して、必要な合計スペースを決定します。
    uint32_t StaleParams = staleRootParamsBitMap_;
    while (_BitScanForward((unsigned long*)&rootIndex, StaleParams)) {
        rootIndices[staleParamCount] = rootIndex;
        StaleParams ^= (1 << rootIndex);

        uint32_t MaxSetHandle{};
        assert(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, rootDescriptorTable_[rootIndex].AssignedHandlesBitMap) &&
            "Root entry marked as stale but has no stale descriptors");

        neededSpace += MaxSetHandle + 1;
        tableSize[staleParamCount] = MaxSetHandle + 1;

        ++staleParamCount;
    }

    assert(staleParamCount <= DescriptorHandleCache::kMaxNumDescriptorTables &&
        "We're only equipped to handle so many descriptor tables");

    staleRootParamsBitMap_ = 0;

    static const uint32_t kMaxDescriptorsPerCopy = 16;
    UINT NumDestDescriptorRanges = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
    UINT pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

    UINT NumSrcDescriptorRanges = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
    UINT pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

    for (uint32_t i = 0; i < staleParamCount; ++i) {
        rootIndex = rootIndices[i];
        (cmdList->*setFunc)(rootIndex, destHandleStart);

        DescriptorTableCache& RootDescTable = rootDescriptorTable_[rootIndex];

        D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandles = RootDescTable.TableStart;
        uint64_t SetHandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
        D3D12_CPU_DESCRIPTOR_HANDLE CurDest = destHandleStart;
        destHandleStart += tableSize[i] * descriptorSize;

        unsigned long SkipCount;
        while (_BitScanForward64(&SkipCount, SetHandles)) {
            // 設定されていない記述子ハンドルをスキップする
            SetHandles >>= SkipCount;
            SrcHandles += SkipCount;
            CurDest.ptr += SkipCount * descriptorSize;

            unsigned long DescriptorCount;
            _BitScanForward64(&DescriptorCount, ~SetHandles);
            SetHandles >>= DescriptorCount;

            // 仮部屋が足りなくなったら、今までのものをコピーします。
            if (NumSrcDescriptorRanges + DescriptorCount > kMaxDescriptorsPerCopy) {
                GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptors(
                    NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
                    NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
                    type);

                NumSrcDescriptorRanges = 0;
                NumDestDescriptorRanges = 0;
            }

            // 宛先範囲の設定
            pDestDescriptorRangeStarts[NumDestDescriptorRanges] = CurDest;
            pDestDescriptorRangeSizes[NumDestDescriptorRanges] = DescriptorCount;
            ++NumDestDescriptorRanges;

            // ソース範囲を設定します（連続しているとは想定していないため、それぞれ 1 つの記述子です）
            for (uint32_t j = 0; j < DescriptorCount; ++j) {
                pSrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandles[j];
                pSrcDescriptorRangeSizes[NumSrcDescriptorRanges] = 1;
                ++NumSrcDescriptorRanges;
            }

            // コピー先のポインタをコピーする記述子の数だけ前方に移動します。
            SrcHandles += DescriptorCount;
            CurDest.ptr += DescriptorCount * descriptorSize;
        }
    }

    GraphicsCore::gGraphicsDevice->GetDevice()->CopyDescriptors(
        NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
        NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
        type);
}

void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid() {
    staleRootParamsBitMap_ = 0;

    unsigned long tableParams = rootDescriptorTablesBitMap_;
    unsigned long rootIndex;
    while (_BitScanForward(&rootIndex, tableParams)) {
        tableParams ^= (1 << rootIndex);
        if (rootDescriptorTable_[rootIndex].AssignedHandlesBitMap != 0)
            staleRootParamsBitMap_ |= (1 << rootIndex);
    }
}

void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]) {
    assert(((1 << RootIndex) & rootDescriptorTablesBitMap_) != 0 && "Root parameter is not a CBV_SRV_UAV descriptor table");
    assert(Offset + NumHandles <= rootDescriptorTable_[RootIndex].TableSize);

    DescriptorTableCache& TableCache = rootDescriptorTable_[RootIndex];
    D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCache.TableStart + Offset;
    for (UINT i = 0; i < NumHandles; ++i)
        CopyDest[i] = Handles[i];
    TableCache.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
    staleRootParamsBitMap_ |= (1 << RootIndex);

}

void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE type, const RootSignature& rootSig) {
    UINT CurrentOffset = 0;

    assert(rootSig.numParameters_ <= 16 && "Maybe we need to support something greater");

    staleRootParamsBitMap_ = 0;
    rootDescriptorTablesBitMap_ = (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?
        rootSig.samplerTableBitMap_ : rootSig.descriptorTableBitMap_);

    unsigned long TableParams = rootDescriptorTablesBitMap_;
    unsigned long RootIndex;
    while (_BitScanForward(&RootIndex, TableParams)) {
        TableParams ^= (1 << RootIndex);

        UINT TableSize = rootSig.descriptorTableSize_[RootIndex];
        assert(TableSize > 0);

        DescriptorTableCache& RootDescriptorTable = rootDescriptorTable_[RootIndex];
        RootDescriptorTable.AssignedHandlesBitMap = 0;
        RootDescriptorTable.TableStart = handleCache_ + CurrentOffset;
        RootDescriptorTable.TableSize = TableSize;

        CurrentOffset += TableSize;
    }

    maxCachedDescriptors_ = CurrentOffset;

    assert(maxCachedDescriptors_ <= kMaxNumDescriptors && "Exceeded user-supplied maximum cache size");
}


}