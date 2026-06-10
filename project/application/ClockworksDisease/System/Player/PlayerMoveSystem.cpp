#include "PlayerMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"

using namespace std;

void PlayerMoveSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	No::Quaternion cameraRotate{};

	auto cameraView = registry.View<FollowCameraComponent>();
	for (auto entity : cameraView) {
		cameraRotate = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}



	auto view = registry.View<PlayerComponent>();
	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent < No::VelocityComponent>(entity);
		velocity->linear = No::Vector3::ZERO;

		// 入力による移動を行います
		
		velocity->linear.x = No::GetInputAxisValue("Horizontal");
		velocity->linear.z = No::GetInputAxisValue("Forward");

		// 入力がされていた場合
		if (velocity->linear.x || velocity->linear.z) {
			
			No::Vector3 rotateVelocity = cameraRotate.RotateVector(velocity->linear);
			velocity->linear.x = rotateVelocity.x;
			velocity->linear.z = rotateVelocity.z;

			// 移動成分を正規化
			velocity->linear = velocity->linear.Normalize();

			velocity->linear *= playerVariables->moveSpeed ;

			// プレイヤーを移動方向へ向ける
			transform->rotation.LookRotation(velocity->linear, No::Vector3::UP);

		}

		// jump(後にSystemを分ける)

		if (No::InputIsTrigger("Jump")) {
			if (registry.GetComponent<No::GroundStateComponent>(entity)->isGrounded || playerVariables->infinityJump) {
				playerVariables->yVelocity = playerVariables->jumpSpeed;
			}
		}

		playerVariables->yVelocity += playerVariables->gravity * deltaTime;

		velocity->linear.y = playerVariables->yVelocity;

		


	}
}
