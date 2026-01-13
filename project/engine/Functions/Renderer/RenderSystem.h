#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Functions/Shader/ShaderModule.h"

namespace NoEngine {
namespace Renderer {
void Initialize();
extern GraphicsPSO gDefaultSpritePSO;
extern RootSignature gDefaultSpriteRootSignature;


}
}