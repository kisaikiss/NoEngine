#include "DebugCameraSystem.h"
#include "../Component/CameraComponent.h"

#include "engine/Functions/Input/Input.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"

namespace NoEngine {
namespace ECS {
void DebugCameraSystem::Update(Registry& registry, float deltaTime) {
	using namespace Component;

	auto view = registry.View<ActiveCameraTag, DebugCameraComponent>();

	for (auto entity : view) {
		auto* debugCamera = registry.GetComponent<DebugCameraComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);

		if (Input::Keyboard::IsPress(VK_LSHIFT)) {
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
				velocity = transform->MakeAffineMatrix4x4().TransformNormal(velocity);

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
			debugCamera->distance += (debugCamera->preMouseWheelY - debugCamera->mouseWheelY) / 500.0f * debugCamera->distance;
			if (debugCamera->distance < 0.0f) {
				debugCamera->distance = 0.0f;
			}
		} else {
			Math::Vector3 velocity{};
			//距離が遠いほど早く移動できる
			velocity.z = (debugCamera->mouseWheelY - debugCamera->preMouseWheelY) / 100.0f;
			//速度ベクトルを自機の向きに合わせて回転させる
			velocity = transform->MakeAffineMatrix4x4().TransformNormal(velocity);

			debugCamera->center += velocity;
		}

		transform->translate.x = debugCamera->center.x + debugCamera->distance * std::sin(debugCamera->phi) * std::cos(debugCamera->theta);
		transform->translate.y = debugCamera->center.y + debugCamera->distance * std::cos(debugCamera->phi);
		transform->translate.z = debugCamera->center.z + debugCamera->distance * std::sin(debugCamera->phi) * std::sin(debugCamera->theta);
		
		Math::Vector3 forward = debugCamera->center - transform->GetWorldPosition();

		transform->rotation.LookRotation(forward.Normalize(), Math::Vector3::UP);

		if (debugCamera->drawCenter) {
			Primitive::DrawSphere(debugCamera->center, 0.3f, Math::Color::WHITE);
		}

	}
}

}
}