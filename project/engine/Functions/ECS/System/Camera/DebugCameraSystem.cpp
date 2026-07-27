#include "DebugCameraSystem.h"
#include "../../Component/Common/CameraComponent.h"

#include "engine/Functions/Input/Input.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Editor/EditUtils.h"

namespace NoEngine {
namespace ECS {
using namespace Component;
void DebugCameraSystem::Update(Registry& registry, float deltaTime) {
#ifndef USE_IMGUI
	return;
#endif // !USE_IMGUI

	if (!Editor::IsMouseOverSceneWindow()) return;
	auto view = registry.View<DebugCameraComponent, TransformComponent>();

	for (auto entity : view) {
		auto* debugCamera = registry.GetComponent<DebugCameraComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);

		// ======= モード切り替え時のパラメータ同期（逆算）処理 =======
		if (debugCamera->moveType != debugCamera->preMoveType) {

			if (debugCamera->moveType == NoEngine::DebugCameraType::kBlender) {
				// 【Unity -> Blender への同期】
				// Unityモードのカメラ位置・向きから、Blenderの注視点(center)と極座標(theta, phi)を逆算する

				// 現在のカメラの前方ベクトルを取得 (Z軸ベクトルを現在の回転で変換)
				Math::Vector3 forward{};
				forward.z = 1.0f;
				forward = transform->MakeAffineMatrix4x4(registry).TransformNormal(forward);
				forward = forward.Normalize();

				// Blenderの極座標を逆算 (カメラから注視点への方向ベクトル V = -forward)
				Math::Vector3 v{};
				v.x = -forward.x;
				v.y = -forward.y;
				v.z = -forward.z;

				// phi (天頂角: 0 ~ PI) を計算。acosがNaNを返さないように念のため-1.0～1.0にクランプ
				float clampY = v.y;
				if (clampY > 1.0f) clampY = 1.0f;
				if (clampY < -1.0f) clampY = -1.0f;
				debugCamera->phi = std::acos(clampY);

				// theta (方位角) を計算
				debugCamera->theta = std::atan2(v.z, v.x);

			} else if (debugCamera->moveType == NoEngine::DebugCameraType::kUnreal) {
				// 【Blender -> Unity への同期】
				// Blenderモードのカメラ位置と注視点から、Unityの角度(Yaw, Pitch)を逆算する

				// 1. 現在の前方ベクトルを計算
				Math::Vector3 forward{};
				forward.x = debugCamera->center.x - transform->translate.x;
				forward.y = debugCamera->center.y - transform->translate.y;
				forward.z = debugCamera->center.z - transform->translate.z;
				forward = forward.Normalize();

				// 2. Yaw (Y軸回転: 水平面の角度) を逆算
				debugCamera->unityYaw = std::atan2(forward.x, forward.z);

				// 3. Pitch (X軸回転: 垂直面の角度) を逆算
				float xzLen = std::sqrt(forward.x * forward.x + forward.z * forward.z);
				debugCamera->unityPitch = std::atan2(-forward.y, xzLen);
			}

			// 状態を更新
			debugCamera->preMoveType = debugCamera->moveType;
		}
		// ==========================================================

		// 実際の移動処理
		switch (debugCamera->moveType) {
		case NoEngine::DebugCameraType::kBlender:
			BlenderMove(registry, entity, deltaTime);
			break;
		case NoEngine::DebugCameraType::kUnreal:
			UnityMove(registry, entity, deltaTime);
			break;
		}


		if (debugCamera->drawCenter) {
			DebugPrimitive::DrawSphere(debugCamera->center, 0.3f, Math::Color::WHITE);
		}
	}
}

void DebugCameraSystem::BlenderMove(Registry& registry, Entity entity, float deltaTime) {
	auto* debugCamera = registry.GetComponent<DebugCameraComponent>(entity);
	auto* transform = registry.GetComponent<TransformComponent>(entity);

	if (Input::Keyboard::IsPress(VK_LSHIFT)) {
		debugCamera->preMousePositionX = debugCamera->mousePositionX;
		debugCamera->preMousePositionY = debugCamera->mousePositionY;
		if (Input::Mouse::IsPress(Input::MouseButton::Middle)) {
			if (Input::Mouse::IsTrigger(Input::MouseButton::Middle)) {
				debugCamera->mousePositionX = 0;
				debugCamera->mousePositionY = 0;
				debugCamera->preMousePositionX = 0;
				debugCamera->preMousePositionY = 0;
			} else {

				Input::Mouse::GetPosition(&debugCamera->mousePositionX, &debugCamera->mousePositionY);
			}
		}

		if (debugCamera->preMousePositionX && debugCamera->mousePositionX) {
			Math::Vector3 velocity{};

			if (debugCamera->isDistanceMove) {
				//距離が遠いほど早く移動できる
				velocity.x = (static_cast<float>(debugCamera->preMousePositionX - debugCamera->mousePositionX) / 20.f * debugCamera->distance) * deltaTime;
				velocity.y = (-static_cast<float>(debugCamera->preMousePositionY - debugCamera->mousePositionY) / 20.f * debugCamera->distance) * deltaTime;
			} else {
				velocity.x = (static_cast<float>(debugCamera->preMousePositionX - debugCamera->mousePositionX) / 2.f) * deltaTime;
				velocity.y = (-static_cast<float>(debugCamera->preMousePositionY - debugCamera->mousePositionY) / 2.f) * deltaTime;
			}

			//速度ベクトルを自機の向きに合わせて回転させる
			velocity = transform->MakeAffineMatrix4x4(registry).TransformNormal(velocity);

			debugCamera->center += velocity;
		}


	} else {
		if (Input::Mouse::IsPress(Input::MouseButton::Middle)) {

			if (Input::Mouse::IsTrigger(Input::MouseButton::Middle)) {
				debugCamera->mousePositionX = 0;
				debugCamera->mousePositionY = 0;
				debugCamera->preMousePositionX = 0;
				debugCamera->preMousePositionY = 0;
			} else {
				debugCamera->preMousePositionX = debugCamera->mousePositionX;
				debugCamera->preMousePositionY = debugCamera->mousePositionY;
				Input::Mouse::GetPosition(&debugCamera->mousePositionX, &debugCamera->mousePositionY);
			}

			if (debugCamera->preMousePositionX && debugCamera->mousePositionX) {
				//カメラの回転はXとY反転させた方が直感的

				debugCamera->theta += (static_cast<float>(debugCamera->preMousePositionX - debugCamera->mousePositionX) / 5.f) * deltaTime;
				debugCamera->phi += (static_cast<float>(debugCamera->preMousePositionY - debugCamera->mousePositionY) / 5.f) * deltaTime;

				//phiの回転に制限を掛けるが、0やpiぴったりだと少し変になるので少し数値をずらす
				float offset = 0.0001f;
				if (debugCamera->phi < offset) {
					debugCamera->phi = offset;
				} else if (debugCamera->phi > std::numbers::pi_v<float> -offset) {
					debugCamera->phi = std::numbers::pi_v<float> -offset;
				}
			}
		}
	}

	debugCamera->preMouseWheelY = debugCamera->mouseWheelY;
	debugCamera->mouseWheelY = Input::Mouse::GetMouseWheelY();

	if (debugCamera->isDistanceMove) {
		//ホイールで奥や手前に移動できる
		//球面座標系での中心点からの距離が遠いほど早く移動できる
		float deltaMouseWheel = debugCamera->preMouseWheelY - debugCamera->mouseWheelY;
		if (std::fabs(deltaMouseWheel) < 140.f) {
			debugCamera->distance += (deltaMouseWheel) / 500.0f * debugCamera->distance;
			if (debugCamera->distance < 0.1f) {
				debugCamera->distance = 0.1f;
			}
		}

	} else {
		Math::Vector3 velocity{};
		//距離が遠いほど早く移動できる
		velocity.z = (debugCamera->mouseWheelY - debugCamera->preMouseWheelY) / 100.0f;
		//速度ベクトルを自機の向きに合わせて回転させる
		velocity = transform->MakeAffineMatrix4x4(registry).TransformNormal(velocity);

		debugCamera->center += velocity;
	}

	transform->translate.x = debugCamera->center.x + debugCamera->distance * std::sin(debugCamera->phi) * std::cos(debugCamera->theta);
	transform->translate.y = debugCamera->center.y + debugCamera->distance * std::cos(debugCamera->phi);
	transform->translate.z = debugCamera->center.z + debugCamera->distance * std::sin(debugCamera->phi) * std::sin(debugCamera->theta);

	Math::Vector3 forward = debugCamera->center - transform->GetWorldPosition(registry);

	transform->rotation.LookRotation(forward.Normalize(), Math::Vector3::UP);

}

void DebugCameraSystem::UnityMove(Registry& registry, Entity entity, float deltaTime) {
	if (Input::Mouse::IsPress(Input::MouseButton::Right)) {
		auto* debugCamera = registry.GetComponent<DebugCameraComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);

		// 1. 右クリックした瞬間に現在のマウス位置で初期化（初動のカメラの飛びを防止）
		if (Input::Mouse::IsTrigger(Input::MouseButton::Right)) {
			Input::Mouse::GetPosition(&debugCamera->mousePositionX, &debugCamera->mousePositionY);
			debugCamera->preMousePositionX = debugCamera->mousePositionX;
			debugCamera->preMousePositionY = debugCamera->mousePositionY;
		} else {
			debugCamera->preMousePositionX = debugCamera->mousePositionX;
			debugCamera->preMousePositionY = debugCamera->mousePositionY;
			Input::Mouse::GetPosition(&debugCamera->mousePositionX, &debugCamera->mousePositionY);
		}

		// マウスの移動量（差分）を計算
		float deltaX = static_cast<float>(debugCamera->mousePositionX - debugCamera->preMousePositionX);
		float deltaY = static_cast<float>(debugCamera->mousePositionY - debugCamera->preMousePositionY);

		// マウスの回転感度
		const float sensitivity = debugCamera->unitySensitivity * deltaTime;

		// 2. 回転角（ラジアン）の更新
		debugCamera->unityYaw += deltaX * sensitivity;
		debugCamera->unityPitch += deltaY * sensitivity;

		// 3. 上下の回転制限（ジンバルロック・画面反転の防止）
		// 真上・真下（約 ±90度 ≒ ±1.57rad）に近づきすぎないようにクランプ
		constexpr float maxPitch = 1.55f;
		if (debugCamera->unityPitch > maxPitch)  debugCamera->unityPitch = maxPitch;
		if (debugCamera->unityPitch < -maxPitch) debugCamera->unityPitch = -maxPitch;

		// 4. クォータニオンを別々に生成して合成し、トランスフォームに適用
		Math::Quaternion qYaw{}, qPitch{};
		qYaw.FromAxisAngle(Math::Vector3::UP, debugCamera->unityYaw);
		qPitch.FromAxisAngle(Math::Vector3::RIGHT, debugCamera->unityPitch);
		transform->rotation = qYaw * qPitch; // ヨー回転の後にピッチ回転を適用（FPSスタイル）

		// 5. 移動速度の計算（WASD）
		Math::Vector3 velocity{};
		const float& unitySpeed = debugCamera->unityMoveSpeed;

		if (Input::Keyboard::IsPress('W')) {
			velocity.z += unitySpeed;
		}
		if (Input::Keyboard::IsPress('S')) {
			velocity.z -= unitySpeed;
		}
		if (Input::Keyboard::IsPress('A')) {
			velocity.x -= unitySpeed;
		}
		if (Input::Keyboard::IsPress('D')) {
			velocity.x += unitySpeed;
		}

		// 速度ベクトルを現在のカメラの向きに合わせて回転（ローカル座標 ➔ ワールド座標）
		velocity = transform->MakeAffineMatrix4x4(registry).TransformNormal(velocity);

		// Q/E による世界座標系（垂直方向）の上下移動（UnityのSceneビューと同じ仕様）
		if (Input::Keyboard::IsPress('E')) {
			velocity.y += unitySpeed;
		}
		if (Input::Keyboard::IsPress('Q')) {
			velocity.y -= unitySpeed;
		}

		// 最終的な座標の更新
		transform->translate += velocity * deltaTime;

		// 現在のカメラの前方ベクトルを取得 (Z軸ベクトルを現在の回転で変換)
		Math::Vector3 forward{};
		forward.z = 1.0f;
		forward = transform->MakeAffineMatrix4x4(registry).TransformNormal(forward);
		forward = forward.Normalize();

		// Blenderの注視点を計算 (現在のカメラ位置から前方に distance 分進んだ位置)
		debugCamera->center.x = transform->translate.x + forward.x * debugCamera->distance;
		debugCamera->center.y = transform->translate.y + forward.y * debugCamera->distance;
		debugCamera->center.z = transform->translate.z + forward.z * debugCamera->distance;
	}
}

}
}