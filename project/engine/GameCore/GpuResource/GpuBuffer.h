#pragma once
#include "GpuResource.h"


//前方宣言

class UploadBuffer;

namespace NoEngine {
class GpuBuffer : public GpuResource {
public:
	virtual ~GpuBuffer() { Destroy(); }

    // バッファを作成します。初期データが指定されている場合は、デフォルトのコマンドコンテキストを使用してバッファにコピーされます。
    void Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize,
        const void* initialData = nullptr);

   /* void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
        const UploadBuffer& srcData, uint32_t srcOffset = 0);*/

    // 事前に割り当てられたヒープからバッファをサブ割り当てします。初期データが指定されている場合は、デフォルトのコマンドコンテキストを使用してバッファにコピーされます。
   /* void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
        const void* initialData = nullptr);*/
private:

protected:

    GpuBuffer(void) : bufferSize_(0), elementCount_(0), elementSize_(0) {
        resourceFlags_ = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        uav_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        srv_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    D3D12_RESOURCE_DESC DescribeBuffer(void);
    virtual void CreateDerivedViews(void) = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE uav_;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_;

    size_t bufferSize_;
    uint32_t elementCount_;
    uint32_t elementSize_;
    D3D12_RESOURCE_FLAGS resourceFlags_;
};
}