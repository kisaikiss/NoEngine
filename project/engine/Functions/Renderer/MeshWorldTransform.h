#pragma once
#include "engine/Math/Types/Matrix4x4.h"
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine {
namespace ECS { class Registry; }
namespace Component {
struct BillboardComponent;
struct CameraComponent;
}

// 配置用Transformを書き換えずに、カメラ依存の描画姿勢を作る。
Math::Matrix4x4 MakeBillboardWorldMatrix(const Math::Matrix4x4& placementWorld,
    const Math::Vector3& worldScale, const Component::BillboardComponent& billboard,
    const Component::CameraComponent* camera);

// 通常描画・輪郭・レイトレーシングで共通のメッシュ行列を使う。
Math::Matrix4x4 MakeMeshWorldMatrix(ECS::Registry& registry, ECS::Entity entity,
    const Component::CameraComponent* renderCamera, const Component::CameraComponent* activeCamera);
}
