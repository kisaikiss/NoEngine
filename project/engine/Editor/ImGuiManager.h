#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

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

