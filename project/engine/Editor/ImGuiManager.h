#pragma once

#include "engine/Runtime/Command/GraphicsContext.h"

namespace NoEngine {
namespace Editor {
class ImGuiManager {
public:
	void Initialize();
	void BeginFrame();
	void Render(GraphicsContext& context);
	void Shutdown();
};
}
}

