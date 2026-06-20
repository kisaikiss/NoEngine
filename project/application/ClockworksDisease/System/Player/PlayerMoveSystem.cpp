#include "PlayerMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"

using namespace std;

void PlayerMoveSystem::Update(No::Registry& registry, float deltaTime) {

	// カメラの回転を取得
	No::Quaternion cameraRotate{};
	auto cameraView = registry.View<FollowCameraComponent>();
	for (auto entity : cameraView) {
		cameraRotate = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}

	auto view = registry.View<PlayerComponent>();
	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);

		velocity->linear = No::Vector3::ZERO;

		const No::Vector3& groundNormal = playerVariables->groundNormal;
		const bool isGrounded = groundState->isGrounded;

		bool justJumped = false;
		if (No::InputIsTrigger("Jump")) {
			if (isGrounded || playerVariables->infinityJump) {
				playerVariables->yVelocity = playerVariables->jumpSpeed;
				justJumped = true;
				// ジャンプ直後は接地フラグを即座に落とす
				// → このフレームの velocity.y 計算を「空中ブランチ」で処理させる
				groundState->isGrounded = false;
			}
		}

		// ------------------------------------------------------------------
		//  水平移動入力
		// ------------------------------------------------------------------
		No::Vector3 inputDir = No::Vector3::ZERO;
		inputDir.x = No::GetInputAxisValue("Horizontal");
		inputDir.z = No::GetInputAxisValue("Forward");

		// 斜面に沿った y 寄与分（斜面下り用）
		float slopeY = 0.f;

		if (inputDir.x != 0.f || inputDir.z != 0.f) {
			particleEmitter->active = true;
			// カメラ基準の移動ベクトル（水平成分のみ）
			No::Vector3 worldDir = cameraRotate.RotateVector(inputDir);
			worldDir.y = 0.f;
			if (isGrounded && !justJumped) {
				float dn = worldDir.Dot(groundNormal);
				worldDir = worldDir - groundNormal * dn;
				// 投影後の y 成分を斜面寄与として取り出す
				// （正規化 → speedをかけた後の y を使うため、ここでは比率のみ保持）
			}

			// 正規化してスピードを乗算
			float len = worldDir.Length();
			if (len > 1e-6f) {
				worldDir = worldDir * (playerVariables->moveSpeed / len);
			}

			velocity->linear.x = worldDir.x;
			velocity->linear.z = worldDir.z;

			// 斜面寄与の y 成分を記録（後で velocity.y に合成）
			slopeY = worldDir.y;

			// プレイヤーを移動方向へ向ける（水平方向のみで LookRotation）
			No::Vector3 lookDir = { worldDir.x, 0.f, worldDir.z };
			if (lookDir.Length() > 1e-6f) {
				No::Quaternion newRotation;
				newRotation.LookRotation(lookDir, groundNormal);
				const float kSlerpScale = 20.f;
				transform->rotation = transform->rotation.Slerp(transform->rotation, newRotation, deltaTime * kSlerpScale);
			}
		} else {
			particleEmitter->active = false;
		}

		if (isGrounded && !justJumped) {
			// 接地中は重力加算しない。

			// 斜面 y のみ velocity に反映
			velocity->linear.y = slopeY;

		} else {
			// 空中 or ジャンプ直後
			playerVariables->yVelocity += playerVariables->gravity * deltaTime;
			velocity->linear.y = playerVariables->yVelocity;
			particleEmitter->active = false;
		}

		playerVariables->groundNormal = No::Vector3::ZERO;
	}
}
