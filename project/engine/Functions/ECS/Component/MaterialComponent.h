#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Component {
struct MaterialComponent {
	Rect uv;
	Color color{ Color::WHITE };
	TextureRef textureHandle;
	GraphicsPSO* pso = nullptr;
};
}
}