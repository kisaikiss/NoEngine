#pragma once
#include "engine/Math/Types/Matrix4x4.h"
#include "../../Entity.h"
namespace NoEngine {
enum class DebugCameraType {
	kBlender,
	kUnreal,
	kUnknown,
};
namespace Component {
/// <summary>
/// 使用中のカメラを示すタグ
/// </summary>
struct ActiveCameraTag {};
struct ActiveCamera2DTag {};

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
	ECS::Entity entity;
	ECS::Entity pad[3];

	Math::Matrix4x4 view;
	Math::Matrix4x4 projection;
	CameraForGPU forGPU;
};

struct Camera2DComponent {
	float width = 1280.f;
	float height = 720.f;
	float zNear = 0.f;
	float zFar = 100.f;

	Math::Matrix4x4 projection;
	Math::Matrix4x4 viewProjection;
};

/// <summary>
/// デバッグカメラを表すコンポーネント
/// </summary>
struct DebugCameraComponent {
	DebugCameraType moveType = DebugCameraType::kBlender;
	NoEngine::DebugCameraType preMoveType = NoEngine::DebugCameraType::kUnknown;
	int preMousePositionX = 0;
	int preMousePositionY = 0;
	int mousePositionX = 0;
	int mousePositionY = 0;
	float preMouseWheelY = 0.0f;
	float mouseWheelY = 0.0f;

	// マウスホイールで球面座標系での距離を移動させるか、中心点を移動させるかを決めるフラグ
	bool isDistanceMove = true;
	// 球面座標系での中心点を描画するか
	bool drawCenter = false;

	// 球面座標系での球面の中心点
	Math::Vector3 center;
	// 球面座標での中心からの距離
	float distance = 2.f;
	// 横回転
	float theta = -1.5f;
	// 縦回転
	float phi = 1.5f;

	float unityYaw = 0.0f;
	float unityPitch = 0.0f;
	float unityMoveSpeed = 10.0f;
	float unitySensitivity = 0.2f;
};

struct DebugCamera2DComponent {
	int preMousePositionX = 0;
	int preMousePositionY = 0;
	int mousePositionX = 0;
	int mousePositionY = 0;
	float preMouseWheelY = 0.0f;
	float mouseWheelY = 0.0f;
};

}
}