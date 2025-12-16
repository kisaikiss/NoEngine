#include "LinearAllocatorPageManager.h"

namespace NoEngine {
LinearAllocatorType LinearAllocatorPageManager::sAutoType = LinearAllocatorType::kGpuExclusive;

LinearAllocatorPageManager::LinearAllocatorPageManager() {
    allocationType_ = sAutoType;
    sAutoType = static_cast<LinearAllocatorType>(static_cast<uint32_t>(sAutoType) + 1);
    assert(sAutoType <= LinearAllocatorType::kNumAllocatorTypes);
}

}
