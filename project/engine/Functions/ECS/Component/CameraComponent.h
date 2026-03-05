#pragma once
#include "engine/Math/Types/Matrix4x4.h"
namespace NoEngine {
namespace Component {
/// <summary>
/// 使用中のカメラを示すタグ
/// </summary>
struct ActiveCameraTag{};

struct CameraForGPU {
    Math::Matrix4x4 viewProjection;
    Math::Vector3 worldPosition;
    float fov = 0.45f;
};
/// <summary>
/// カメラを表すコンポーネント
/// </summary>
struct CameraComponent {
    float fov = 0.45f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    float aspect = 16.0f / 9.0f;

    Math::Matrix4x4 view;
    Math::Matrix4x4 projection;
    CameraForGPU forGPU;
};
}
}