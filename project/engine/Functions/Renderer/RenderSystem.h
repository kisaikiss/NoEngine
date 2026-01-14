#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/DescriptorHeap/DescriptorHeap.h"
#include "engine/Functions/Shader/ShaderModule.h"

namespace NoEngine {
namespace Render {
void Initialize();
void Shutdown();
extern DescriptorHeap gTextureHeap;

GraphicsPSO& GetPSO(std::wstring psoName);
RootSignature& GetRootSignature(std::string rootSigName);
}
}