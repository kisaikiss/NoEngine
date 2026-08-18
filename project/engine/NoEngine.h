#pragma once
#include "Runtime/GameCore.h"

#include "Functions/ECS/Component/Asset/MeshComponent.h"
#include "Functions/ECS/Component/Asset/Animator2DComponent.h"
#include "Functions/ECS/Component/Asset/SpriteComponent.h"
#include "Functions/ECS/Component/Asset/TextComponent.h"
#include "Functions/ECS/Component/Common/TransformComponent.h"
#include "Functions/ECS/Component/Common/Transform2DComponent.h"
#include "Functions/ECS/Component/Asset/MaterialComponent.h"
#include "Functions/ECS/Component/Asset/LightComponent.h"
#include "Functions/ECS/Component/Common/CameraComponent.h"
#include "Functions/ECS/Component/Effect/ParticleEmitterComponent.h"
#include "Functions/ECS/Component/Common/VelocityComponent.h"
#include "Functions/ECS/Component/Effect/EffectEmitterComponent.h"
#include "Functions/ECS/Event/SceneChangeEvent.h"
#include "Editor/EditUtils.h"
#include "Editor/DataDriven/PrefabSerializer.h"

#include "Functions/ECS/System/Loader/SpriteLoadSystem.h"
#include "Functions/ECS/System/Loader/ModelLoadSystem.h"
#include "Functions/ECS/System/Animation/AnimationSystem.h"
#include "Functions/ECS/System/Animation/SpriteAnimationSystem.h"
#include "Functions/ECS/System/Animation/TransformRoutineSystem.h"
#include "Functions/ECS/System/Editor/EditSystem.h"
#include "Functions/ECS/System/Editor/DrawManipulatorSystem.h"
#include "Functions/ECS/System/Editor/DrawWaypointRouteSystem.h"
#include "Functions/ECS/System/Editor/DrawCameraFrustumSystem.h"
#include "Functions/ECS/System/Camera/CameraSystem.h"
#include "Functions/ECS/System/Camera/Camera2DSystem.h"
#include "Functions/ECS/System/Camera/DebugCameraSystem.h"
#include "Functions/ECS/System/Camera/DebugCamera2DSystem.h"
#include "Functions/ECS/System/Movement/MovementSystem.h"
#include "Functions/ECS/System/Movement/Movement2DSystem.h"
#include "Functions/ECS/System/Movement/CapturePlatformPrevTransformSystem.h"
#include "Functions/ECS/System/Movement/ComputePlatformDeltaSystem.h"
#include "Functions/ECS/System/Collision/CollisionResolutionSystem.h"
#include "Functions/ECS/System/Collision/NarrowPhaseSystem.h"
#include "Functions/ECS/System/Collision/CollisionResolution2DSystem.h"
#include "Functions/ECS/System/Collision/NarrowPhase2DSystem.h"
#include "Functions/ECS/System/Effect/ParticleSystem.h"
#include "Functions/ECS/System/Effect/ParticleEmitterSystem.h"
#include "Functions/ECS/System/Movement/GroundResetSystem.h"
#include "Functions/ECS/System/Effect/EffectEmitSystem.h"

#include "Assets/Audio/Audio.h"
#include "Assets/AssetManager.h"
#include "Functions/Renderer/RaytracingManager.h"
#include "Functions/Renderer/Primitive.h"
#include "Functions/Input/Input.h"
#include "Functions/Input/InputConfig.h"

#include "Math/MathInclude.h"

#include "Utilities/Random.h"
#include "Editor/ReflectionMacros.h"

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