#pragma once

namespace NoEngine {
class DescriptorHandle {

public:
    DescriptorHandle() {
        cpuHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        gpuHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    DescriptorHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle )
        : cpuHandle_(cpuHandle)
    {
        gpuHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
        : cpuHandle_(cpuHandle), gpuHandle_(gpuHandle) {
    }

    DescriptorHandle operator+ (INT offsetScaledByDescriptorSize) const {
        DescriptorHandle ret = *this;
        ret += offsetScaledByDescriptorSize;
        return ret;
    }

    void operator += (INT offsetScaledByDescriptorSize) {
        if (cpuHandle_.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            cpuHandle_.ptr += offsetScaledByDescriptorSize;
        if (gpuHandle_.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            gpuHandle_.ptr += offsetScaledByDescriptorSize;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &cpuHandle_; }
    operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return cpuHandle_; }
    operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return gpuHandle_; }

    size_t GetCpuPtr() const { return cpuHandle_.ptr; }
    uint64_t GetGpuPtr() const { return gpuHandle_.ptr; }
    bool IsNull() const { return cpuHandle_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
    bool IsShaderVisible() const { return gpuHandle_.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
};
}