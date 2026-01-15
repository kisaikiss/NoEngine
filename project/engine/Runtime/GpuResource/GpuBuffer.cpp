#include "GpuBuffer.h"
#include "../Command/CommandContext.h"
#include "../GraphicsCore.h"
#include "engine/Math/Common.h"

namespace NoEngine {

void GpuBuffer::Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* initialData) {

    Destroy();

    elementCount_ = numElements;
    elementSize_ = elementSize;
    bufferSize_ = static_cast<size_t>(numElements) * elementSize;

    D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

    usageState_ = D3D12_RESOURCE_STATE_COMMON;

    D3D12_HEAP_PROPERTIES HeapProps{};
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
        &ResourceDesc, usageState_, nullptr, IID_PPV_ARGS(&resource_));
    if (FAILED(hr)) {
        assert(false);
    }
    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();

    if (initialData) {
       CommandContext::InitializeBuffer(*this, initialData, bufferSize_);
    }


#ifdef RELEASE
    (name);
#else
    resource_->SetName(name.c_str());
#endif

    CreateDerivedViews();
}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& srcData, uint32_t srcOffset)
{
    Destroy();

    elementCount_ = NumElements;
    elementSize_ = ElementSize;
    bufferSize_ = NumElements * ElementSize;

    D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

    usageState_ = D3D12_RESOURCE_STATE_COMMON;

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
            &ResourceDesc, usageState_, nullptr, IID_PPV_ARGS(&resource_));
    if (FAILED(hr))
    {
        assert(false);
    }
    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();

    CommandContext::InitializeBuffer(*this, srcData, srcOffset);

#ifdef _DEBUG
    resource_->SetName(name.c_str());
#else
    (name);
#endif

    CreateDerivedViews();
}

D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t offset, uint32_t size) const {
    assert(offset + size <= bufferSize_);
    size = Math::AlignUp(size, 16);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
    cbvDesc.BufferLocation = gpuVirtualAddress_ + static_cast<size_t>(offset);
    cbvDesc.SizeInBytes = size;

    D3D12_CPU_DESCRIPTOR_HANDLE hCbv = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateConstantBufferView(&cbvDesc, hCbv);
    return hCbv;
}

D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t offset, uint32_t size, uint32_t stride) const {
    D3D12_VERTEX_BUFFER_VIEW vbView{};
    vbView.BufferLocation = gpuVirtualAddress_ + offset;
    vbView.SizeInBytes = size;
    vbView.StrideInBytes = stride;
    return vbView;
}

D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t offset, uint32_t size, bool b32Bit) const {
    D3D12_INDEX_BUFFER_VIEW ibView{};
    ibView.BufferLocation = gpuVirtualAddress_ + offset;
    ibView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    ibView.SizeInBytes = size;
    return ibView;
}


D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer(void) const {
    assert(bufferSize_ != 0);

    D3D12_RESOURCE_DESC desc = {};
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Flags = resourceFlags_;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Height = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = (UINT64)bufferSize_;
    return desc;
}

void ByteAddressBuffer::CreateDerivedViews(void) {
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Buffer.NumElements = (UINT)bufferSize_ / 4;
    SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    if (srv_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        srv_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), &SRVDesc, srv_);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    UAVDesc.Buffer.NumElements = (UINT)bufferSize_ / 4;
    UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

    if (uav_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        uav_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateUnorderedAccessView(resource_.Get(), nullptr, &UAVDesc, uav_);
}

void StructuredBuffer::CreateDerivedViews(void) {
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Buffer.NumElements = elementCount_;
    SRVDesc.Buffer.StructureByteStride = elementSize_;
    SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    if (srv_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        srv_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), &SRVDesc, srv_);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    UAVDesc.Buffer.CounterOffsetInBytes = 0;
    UAVDesc.Buffer.NumElements = elementCount_;
    UAVDesc.Buffer.StructureByteStride = elementSize_;
    UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    counterBuffer_.Create(L"StructuredBuffer::Counter", 1, 4);

    if (uav_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        uav_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateUnorderedAccessView(resource_.Get(), counterBuffer_.GetResource(), &UAVDesc, uav_);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& Context) {
    Context.TransitionResource(counterBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
    return counterBuffer_.GetSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& Context) {
    Context.TransitionResource(counterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    return counterBuffer_.GetUAV();
}

void TypedBuffer::CreateDerivedViews(void) {
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    SRVDesc.Format = dataFormat_;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Buffer.NumElements = elementCount_;
    SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    if (srv_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        srv_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), &SRVDesc, srv_);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    UAVDesc.Format = dataFormat_;
    UAVDesc.Buffer.NumElements = elementCount_;
    UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    if (uav_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        uav_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateUnorderedAccessView(resource_.Get(), nullptr, &UAVDesc, uav_);
}

}
