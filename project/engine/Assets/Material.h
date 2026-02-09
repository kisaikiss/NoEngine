#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
struct Material {
	Rect uv{};
	Math::Color color{ Math::Color::WHITE };
	TextureRef textureHandle;
	
};
}