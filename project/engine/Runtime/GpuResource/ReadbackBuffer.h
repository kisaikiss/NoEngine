#pragma once
#include "GpuBuffer.h"
namespace NoEngine {

class ReadbackBuffer : public GpuBuffer {
public:
    virtual ~ReadbackBuffer() { Destroy(); }

    void Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize);

    void* Map(void);
    void Unmap(void);

protected:

    void CreateDerivedViews(void) {}
};
}

