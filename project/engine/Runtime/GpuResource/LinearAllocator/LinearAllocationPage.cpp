#include "LinearAllocationPage.h"

namespace NoEngine {
LinearAllocationPage::LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES usage) : GpuResource() {
    resource_.Attach(pResource);
    usageState_ = usage;
    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();
    resource_->Map(0, nullptr, &cpuVirtualAddress_);
}

LinearAllocationPage::~LinearAllocationPage() {
    Unmap();
}

void LinearAllocationPage::Map(void) {
    if (cpuVirtualAddress_ == nullptr) {
        resource_->Map(0, nullptr, &cpuVirtualAddress_);
    }
}

void LinearAllocationPage::Unmap(void) {
    if (cpuVirtualAddress_ != nullptr) {
        resource_->Unmap(0, nullptr);
        cpuVirtualAddress_ = nullptr;
    }
}
}
