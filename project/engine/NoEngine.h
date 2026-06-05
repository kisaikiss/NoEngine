#pragma once
#include "Runtime/GameCore.h"

#include "Functions/ECS/Component/MeshComponent.h"
#include "Functions/ECS/Component/Animator2DComponent.h"
#include "Functions/ECS/Component/SpriteComponent.h"
#include "Functions/ECS/Component/TransformComponent.h"
#include "Functions/ECS/Component/Transform2DComponent.h"
#include "Functions/ECS/Component/MaterialComponent.h"
#include "Functions/ECS/Component/LightComponent.h"
#include "Functions/ECS/Component/CameraComponent.h"
#include "Functions/ECS/Component/ParticleEmitterComponent.h"
#include "Functions/ECS/Component/VelocityComponent.h"
#include "Functions/ECS/Component/Effect/EffectEmitterComponent.h"
#include "Functions/ECS/Event/SceneChangeEvent.h"
#include "Editor/EditUtils.h"
#include "Editor/ReflectionMacros.h"

#include "Functions/ECS/System/Loader/SpriteLoadSystem.h"
#include "Functions/ECS/System/AnimationSystem.h"
#include "Functions/ECS/System/SpriteAnimationSystem.h"
#include "Functions/ECS/System/Editor/EditSystem.h"
#include "Functions/ECS/System/Editor/DrawManipulatorSystem.h"
#include "Functions/ECS/System/Camera/CameraSystem.h"
#include "Functions/ECS/System/Camera/Camera2DSystem.h"
#include "Functions/ECS/System/Camera/DebugCameraSystem.h"
#include "Functions/ECS/System/Camera/DebugCamera2DSystem.h"
#include "Functions/ECS/System/MovementSystem.h"
#include "Functions/ECS/System/Movement2DSystem.h"
#include "Functions/ECS/System/Collision/CollisionResolutionSystem.h"
#include "Functions/ECS/System/Collision/NarrowPhaseSystem.h"
#include "Functions/ECS/System/Collision/CollisionResolution2DSystem.h"
#include "Functions/ECS/System/Collision/NarrowPhase2DSystem.h"
#include "Functions/ECS/System/Effect/ParticleSystem.h"
#include "Functions/ECS/System/Effect/ParticleEmitterSystem.h"
#include "Functions/ECS/System/GroundResetSystem.h"
#include "Functions/ECS/System/Effect/EffectEmitSystem.h"

#include "Assets/ModelLoader.h"
#include "Assets/Audio/Audio.h"
#include "Functions/Renderer/RenderSystem.h"
#include "Functions/Renderer/Primitive.h"
#include "Functions/Input/Input.h"
#include "Functions/Input/InputConfig.h"

#include "Math/MathInclude.h"

#include "Utilities/Random.h"

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
using namespace NoEngine::Random;
}