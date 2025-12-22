#include "LinearAllocator.h"
#include "engine/Math/Common.h"

namespace NoEngine {

LinearAllocatorPageManager LinearAllocator::sPageManager_[2];
DynAlloc LinearAllocator::Allocate(size_t SizeInBytes, size_t Alignment) {
    const size_t alignmentMask = Alignment - 1;

    assert((alignmentMask & Alignment) == 0);

    // 割り当てを調整します。
    const size_t alignedSize = Math::AlignUpWithMask(SizeInBytes, alignmentMask);

    if (alignedSize > pageSize_)
        return AllocateLargePage(alignedSize);

    curOffset_ = Math::AlignUp(curOffset_, Alignment);

    if (curOffset_ + alignedSize > pageSize_) {
        assert(curPage_ != nullptr);
        retiredPages_.push_back(curPage_);
        curPage_ = nullptr;
    }

    if (curPage_ == nullptr) {
        curPage_ = sPageManager_[static_cast<size_t>(allocationType_)].RequestPage();
        curOffset_ = 0;
    }

    DynAlloc ret(*curPage_, curOffset_, alignedSize);
    ret.DataPtr = (uint8_t*)curPage_->cpuVirtualAddress_ + curOffset_;
    ret.GpuAddress = curPage_->gpuVirtualAddress_ + curOffset_;

    curOffset_ += alignedSize;

    return ret;
}

void LinearAllocator::CleanupUsedPages(uint64_t fenceID) {
    if (curPage_ != nullptr) {
        retiredPages_.push_back(curPage_);
        curPage_ = nullptr;
        curOffset_ = 0;
    }

    sPageManager_[static_cast<size_t>(allocationType_)].DiscardPages(fenceID, retiredPages_);
    retiredPages_.clear();

    sPageManager_[static_cast<size_t>(allocationType_)].FreeLargePages(fenceID, largePageList_);
    largePageList_.clear();
}

DynAlloc LinearAllocator::AllocateLargePage(size_t SizeInBytes) {
    LinearAllocationPage* oneOff = sPageManager_[static_cast<size_t>(allocationType_)].CreateNewPage(SizeInBytes);
    largePageList_.push_back(oneOff);

    DynAlloc ret(*oneOff, 0, SizeInBytes);
    ret.DataPtr = oneOff->cpuVirtualAddress_;
    ret.GpuAddress = oneOff->gpuVirtualAddress_;

    return ret;
}
}