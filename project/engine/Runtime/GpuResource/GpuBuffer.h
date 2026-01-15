#pragma once
#include "GpuResource.h"

namespace NoEngine {
//前方宣言
class UploadBuffer;


class GpuBuffer : public GpuResource {
public:
	virtual ~GpuBuffer() { Destroy(); }

    // バッファを作成します。初期データが指定されている場合は、デフォルトのコマンドコンテキストを使用してバッファにコピーされます。
    void Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize,
        const void* initialData = nullptr);

    void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
        const UploadBuffer& srcData, uint32_t srcOffset = 0);

    // 事前に割り当てられたヒープからバッファをサブ割り当てします。初期データが指定されている場合は、デフォルトのコマンドコンテキストを使用してバッファにコピーされます。
   /* void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
        const void* initialData = nullptr);*/

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return uav_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return srv_; }

    D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView(void) const { return gpuVirtualAddress_; }

    D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t offset, uint32_t size) const;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t offset, uint32_t size, uint32_t stride) const;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t baseVertexIndex = 0) const {
        size_t offset = baseVertexIndex * elementSize_;
        return VertexBufferView(offset, (uint32_t)(bufferSize_ - offset), elementSize_);
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t offset, uint32_t size, bool b32Bit = false) const;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t startIndex = 0) const {
        size_t offset = startIndex * elementSize_;
        return IndexBufferView(offset, (uint32_t)(bufferSize_ - offset), elementSize_ == 4);
    }

    size_t GetBufferSize() const { return bufferSize_; }
    uint32_t GetElementCount() const { return elementCount_; }
    uint32_t GetElementSize() const { return elementSize_; }

protected:

    GpuBuffer(void) : bufferSize_(0), elementCount_(0), elementSize_(0) {
        resourceFlags_ = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        uav_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        srv_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    D3D12_RESOURCE_DESC DescribeBuffer(void) const;

    /// <summary>
    /// 派生ビューの作成を行います。
    /// </summary>
    virtual void CreateDerivedViews(void) = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE uav_;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_;

    size_t bufferSize_;
    uint32_t elementCount_;
    uint32_t elementSize_;
    D3D12_RESOURCE_FLAGS resourceFlags_;
};

class ByteAddressBuffer : public GpuBuffer {
public:
    virtual void CreateDerivedViews(void) override;
};

class IndirectArgsBuffer : public ByteAddressBuffer {
public:
    IndirectArgsBuffer(void) {}
};

class StructuredBuffer : public GpuBuffer {
public:
    virtual void Destroy(void) override {
        counterBuffer_.Destroy();
        GpuBuffer::Destroy();
    }

    virtual void CreateDerivedViews(void) override;

    ByteAddressBuffer& GetCounterBuffer(void) { return counterBuffer_; }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CommandContext& Context);
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CommandContext& Context);

private:
    ByteAddressBuffer counterBuffer_;
};

class TypedBuffer : public GpuBuffer {
public:
    TypedBuffer(DXGI_FORMAT Format) : dataFormat_(Format) {}
    virtual void CreateDerivedViews(void) override;

protected:
    DXGI_FORMAT dataFormat_;
};

}