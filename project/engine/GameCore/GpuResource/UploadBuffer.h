#pragma once
#include "GpuResource.h"
namespace NoEngine {
class UploadBuffer :
    public GpuResource {
public:
    virtual ~UploadBuffer() { Destroy(); }

    void Create(const std::wstring& name, size_t BufferSize);

    void* Map(void);
    void Unmap(size_t begin = 0, size_t end = -1);

    size_t GetBufferSize() const { return bufferSize_; }

protected:

    size_t bufferSize_;
};
}