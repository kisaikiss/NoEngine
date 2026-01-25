#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
struct Material {
	Rect uv{};
	Color color{ Color::WHITE };
	TextureRef textureHandle;
	
};
}