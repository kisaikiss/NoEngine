#pragma once
#include "Runtime/GameCore.h"

#include "Functions/ECS/Component/MeshComponent.h"
#include "Functions/ECS/Component/TransformComponent.h"
#include "Functions/ECS/Component/MaterialComponent.h"

#include "Assets/ModelLoader.h"
#include "Functions/Renderer/RenderSystem.h"
#include "Functions/Input/Keyboard.h"

#include "Math/MathInclude.h"

#ifdef USE_IMGUI
#include <externals/imgui/imgui.h>
#endif // USE_IMGUI


namespace No {
using namespace NoEngine::GameCore;
using namespace NoEngine::ECS;
using namespace NoEngine::Input;
using namespace NoEngine::Component;
using namespace NoEngine::Scene;
}