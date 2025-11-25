#pragma once
#include "LinearAllocationPage.h"
#include "LinearAllocatorType.h"

namespace NoEngine {
/// <summary>
/// ページ単位のメモリブロック管理
/// </summary>
class LinearAllocatorPageManager {

public:

    LinearAllocatorPageManager();
    LinearAllocationPage* RequestPage(void);
    LinearAllocationPage* CreateNewPage(size_t pageSize = 0);

    // 破棄されたページはリサイクルされます。これは固定サイズのページに適用されます。
    void DiscardPages(uint64_t fenceID, const std::vector<LinearAllocationPage*>& pages);

    // 解放されたページは、フェンスを通過すると破棄されます。これは、1回限りの「大きな」ページ用です。
    void FreeLargePages(uint64_t fenceID, const std::vector<LinearAllocationPage*>& pages);

    void Destroy(void) { pagePool_.clear(); }

private:

    static LinearAllocatorType sAutoType;

    LinearAllocatorType allocationType_;
    std::vector<std::unique_ptr<LinearAllocationPage> > pagePool_;
    std::queue<std::pair<uint64_t, LinearAllocationPage*> > retiredPages_;
    std::queue<std::pair<uint64_t, LinearAllocationPage*> > deletionQueue_;
    std::queue<LinearAllocationPage*> availablePages_;
    std::mutex mutex_;
};


}