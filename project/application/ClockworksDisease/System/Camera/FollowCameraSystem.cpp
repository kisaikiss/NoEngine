#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"
#include "../../Component/Game/GameProgressComponent.h"

void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	No::Vector3 playerPos{};
	bool playerGrounded = false;
	auto playerView = registry.View<No::TransformComponent, No::VelocityComponent, No::GroundStateComponent, PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
		playerGrounded = registry.GetComponent<No::GroundStateComponent>(e)->isGrounded;
	}

	auto view = registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>();

	for (auto e : view) {
		if (registry.Has<CameraLockTag>(e)) continue; // 演出中は追従しない
		
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);
		auto* cameraComponent = registry.GetComponent<No::CameraComponent>(e);

		// 通常のジャンプでは高さを保ち、ジャンプ中に一定以上上昇したら空中でも追従する。
		if (!followCameraVariables->followHeightInitialized) {
			followCameraVariables->followHeight = playerPos.y;
			followCameraVariables->followHeightInitialized = true;
			followCameraVariables->followHeightAirborneCatchUp = false;
			followCameraVariables->followHeightCatchUpInProgress = false;
		} else if (playerGrounded) {
			followCameraVariables->followHeightAirborneCatchUp = false;
			followCameraVariables->followHeightCatchUpInProgress = false;
			const float heightDelta = playerPos.y - followCameraVariables->followHeight;
			if (heightDelta > 0.0f) {
				// 着地後の高台追従は目標へ近づくほど減速し、滑らかに止める。
				const float riseAlpha = 1.0f - std::exp(-std::max(followCameraVariables->heightRiseFollowSpeed, 0.0f) * deltaTime);
				followCameraVariables->followHeight = No::Lerp(followCameraVariables->followHeight, playerPos.y, riseAlpha);
			} else if (heightDelta < 0.0f) {
				// 着地して低い場所へ移動した場合も下降速度を使う。
				const float maxFallStep = std::max(followCameraVariables->heightFallFollowSpeed, 0.0f) * deltaTime;
				followCameraVariables->followHeight += std::max(-maxFallStep, heightDelta);
			}
		} else {
			const float riseThreshold = std::max(followCameraVariables->heightRiseCatchUpThreshold, 0.0f);
			if (!followCameraVariables->followHeightAirborneCatchUp
				&& playerPos.y - followCameraVariables->followHeight >= riseThreshold) {
				followCameraVariables->followHeightAirborneCatchUp = true;
				followCameraVariables->followHeightCatchUpInProgress = true;
			}
			if (followCameraVariables->followHeightAirborneCatchUp) {
				if (followCameraVariables->followHeightCatchUpInProgress) {
					// 通常追従へ切り替える前に、追従基準を滑らかにプレイヤーへ近づける。
					const float heightDelta = playerPos.y - followCameraVariables->followHeight;
					const float catchUpSpeed = heightDelta >= 0.0f
						? followCameraVariables->heightRiseFollowSpeed
						: followCameraVariables->heightFallFollowSpeed;
					const float catchUpAlpha = 1.0f - std::exp(-std::max(catchUpSpeed, 0.0f) * deltaTime);
					followCameraVariables->followHeight = No::Lerp(followCameraVariables->followHeight, playerPos.y, catchUpAlpha);
					if (std::abs(playerPos.y - followCameraVariables->followHeight) <= 0.05f) {
						followCameraVariables->followHeight = playerPos.y;
						followCameraVariables->followHeightCatchUpInProgress = false;
					}
				} else {
					// 追いついた後は通常のカメラ位置補間に高さ追従を任せる。
					followCameraVariables->followHeight = playerPos.y;
				}
			} else if (playerPos.y < followCameraVariables->followHeight) {
				// 閾値未満のジャンプで基準高さより下がった場合は、下降方向へ速く追従する。
				const float maxFallStep = std::max(followCameraVariables->heightFallFollowSpeed, 0.0f) * deltaTime;
				followCameraVariables->followHeight += std::max(-maxFallStep, playerPos.y - followCameraVariables->followHeight);
			}
		}
		No::Vector3 cameraFollowPos = playerPos;
		cameraFollowPos.y = followCameraVariables->followHeight;

		float hInput = No::GetInputAxisValue("CameraHorizontal");
		float vInput = No::GetInputAxisValue("CameraVertical");

		followCameraVariables->phi += vInput * deltaTime * followCameraVariables->rotationSpeed;
		followCameraVariables->theta += hInput * deltaTime * followCameraVariables->rotationSpeed;


		const float& minPhi = followCameraVariables->minPhi;
		const float& maxPhi = followCameraVariables->maxPhi;

		// 移動制限
		if (followCameraVariables->phi >= maxPhi) {
			followCameraVariables->phi = maxPhi;
		} else if (followCameraVariables->phi <= minPhi) {
			followCameraVariables->phi = minPhi;
		}

		// Phi(縦方向の移動)によって「プレイヤーとカメラの距離」と「視野角」を調整する
		float normalizedPhi = (followCameraVariables->phi - minPhi) / (maxPhi - minPhi);
		followCameraVariables->distance = No::Lerp(followCameraVariables->maxDistance, followCameraVariables->minDistance, normalizedPhi);
		const float lookUpBlend = std::clamp((normalizedPhi - 0.75f) / 0.25f, 0.0f, 1.0f);
		const float smoothLookUpBlend = lookUpBlend * lookUpBlend * (3.0f - 2.0f * lookUpBlend);
		const float lookUpMinHeight = playerPos.y + std::max(followCameraVariables->lookUpCameraClearance, 0.0f);

		cameraComponent->fov = No::Lerp(followCameraVariables->minFov, followCameraVariables->maxFov, normalizedPhi);

		// 理想位置(衝突を考慮しない、zoom後のdistanceそのまま)
		No::Vector3 idealPos;
		idealPos.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		const float idealCameraY = cameraFollowPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		idealPos.y = No::Lerp(idealCameraY, std::max(idealCameraY, lookUpMinHeight), smoothLookUpBlend);
		idealPos.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		// 注視対象に向けてスイープ（空間走査）を行う際、近接する壁を無視してはならない。
		// プレイヤーの原点は足元にあるため、球体を（上方へ）移動させて、床の中に埋まった状態で開始されるのを防ぐ必要がある。
		const float radius = std::max(followCameraVariables->collisionRadius, 0.001f);
		No::Vector3 anchor = playerPos + No::Vector3{ 0.0f,
			std::max(followCameraVariables->playerPosOffset, radius), 0.0f };
		anchor += No::MoveCapsuleTerrain(registry, NoEngine::CapsuleWorld{ anchor, anchor, radius }, No::Vector3::ZERO);
		auto visiblePosition = [&](const No::Vector3& target) {
			const No::Vector3 path = target - anchor;
			const float length = path.Length();
			if (length <= 1e-6f) return anchor;
			const auto hit = No::SweepCapsuleTerrain(registry,
				NoEngine::CapsuleWorld{ anchor, anchor, radius }, path);
			if (!hit.hit) return target;
			const float distance = std::max(0.0f,
				length * hit.fraction - std::max(followCameraVariables->collisionMargin, 0.0f));
			return anchor + path * (distance / length);
		};

		const No::Vector3 safeIdeal = visiblePosition(idealPos);
		const float targetDistance = (safeIdeal - anchor).Length();
		// すぐに収縮させます。収縮を滑らかにしようとすると、カメラが壁の後ろに隠れてしまいます。
		if (targetDistance < followCameraVariables->currentDistance) {
			followCameraVariables->currentDistance = targetDistance;
		} else {
			const float alpha = 1.0f - std::exp(-4.0f * deltaTime);
			followCameraVariables->currentDistance = No::Lerp(
				followCameraVariables->currentDistance, targetDistance, alpha);
		}
		const No::Vector3 idealPath = idealPos - anchor;
		const float idealLength = idealPath.Length();
		const No::Vector3 nextPosition = idealLength > 1e-6f
			? anchor + idealPath * (followCameraVariables->currentDistance / idealLength)
			: anchor;

		const No::Vector3 nowPosition = transform->GetWorldPosition(registry);
		const float moveTime = 1.0f - std::exp(-followCameraVariables->moveSpeed * deltaTime);
		const No::Vector3 desiredPosition = visiblePosition(No::Lerp(nowPosition, nextPosition, moveTime));
		// 軌道運動と位置補間についても、広範囲にわたるテストを行う。
		const No::Vector3 moved = No::MoveCapsuleTerrain(registry,
			NoEngine::CapsuleWorld{ nowPosition, nowPosition, radius }, desiredPosition - nowPosition);
		transform->SetWorldPosition(registry, visiblePosition(nowPosition + moved));

		followCameraVariables->nextRotation = transform->rotation;
		float viewSpeed = followCameraVariables->playerViewSpeed;
		if (No::InputIsPress("ViewGoal")) {
			// ゴールオブジェクト座標を見る
			for (auto goalItemE : registry.View<GoalItemTag, No::TransformComponent>()) {
				No::Vector3 goalItemPos = registry.GetComponent<No::TransformComponent>(goalItemE)->GetWorldPosition(registry);
				followCameraVariables->nextRotation.LookRotation(goalItemPos - transform->GetWorldPosition(registry), No::Vector3::UP);
			}

			viewSpeed = followCameraVariables->goalViewSpeed;
		} else {
			// プレイヤー + Offset座標を見る
			cameraFollowPos.y += followCameraVariables->playerPosOffset;
			followCameraVariables->nextRotation.LookRotation(cameraFollowPos - transform->GetWorldPosition(registry), No::Vector3::UP);
		}

		const float rotationAlpha = 1.0f - std::exp(-viewSpeed * deltaTime);
		transform->rotation = No::Quaternion::Slerp(transform->rotation, followCameraVariables->nextRotation, rotationAlpha);
	
	}



}

REGISTER_SYSTEM(::FollowCameraSystem, "FollowCameraSystem", "Camera")
