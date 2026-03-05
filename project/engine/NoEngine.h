#pragma once
#include "Runtime/GameCore.h"

#include "Functions/ECS/Component/MeshComponent.h"
#include "Functions/ECS/Component/SpriteComponent.h"
#include "Functions/ECS/Component/TransformComponent.h"
#include "Functions/ECS/Component/Transform2DComponent.h"
#include "Functions/ECS/Component/MaterialComponent.h"
#include "Functions/ECS/Component/LightComponent.h"
#include "Functions/ECS/Component/CameraComponent.h"
#include "Functions/ECS/Event/SceneChangeEvent.h"
#include "Editor/EditUtils.h"
#include "Editor/ReflectionMacros.h"

#include "Functions/ECS/System/AnimationSystem.h"
#include "Functions/ECS/System/EditSystem.h"
#include "Functions/ECS/System/CameraSystem.h"

#include "Assets/ModelLoader.h"
#include "Assets/Audio/Audio.h"
#include "Functions/Renderer/RenderSystem.h"
#include "Functions/Renderer/Primitive.h"
#include "Functions/Input/Input.h"

#include "Math/MathInclude.h"

#ifdef USE_IMGUI
#include <externals/imgui/imgui.h>
#endif // USE_IMGUI


namespace No {
using namespace NoEngine::GameCore;
using namespace NoEngine::ECS;
using namespace NoEngine::Input;
using namespace NoEngine::Component;
using namespace NoEngine::Event;
using namespace NoEngine::Scene;
using namespace NoEngine::Asset;
using namespace NoEngine::Easing;
using namespace NoEngine::Math;
using namespace NoEngine::Editor;
}