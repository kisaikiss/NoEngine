#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"
#include "../../Component/Game/GameProgressComponent.h"

namespace {
constexpr float kAvoidanceHoldTime = 0.4f;
constexpr float kLookAheadTime = 0.25f;
constexpr float kMaxCorrectionSpeed = 16.0f;

No::Vector3 LimitLength(const No::Vector3& value, float maximum) {
	const float length = value.Length();
	return length > maximum && length > 1e-6f ? value * (maximum / length) : value;
}

void AdvanceAvoidanceYaw(float& theta, float& velocity, float target, float maxSpeed, float deltaTime) {
	if (deltaTime <= 0.0f) return;
	const float error = std::remainder(target - theta, 2.0f * PI);
	constexpr float response = 8.0f;
	constexpr float acceleration = 3.0f;
	const float denominator = 1.0f + response * deltaTime;
	float desiredVelocity = (velocity + error * response * response * deltaTime) / (denominator * denominator);
	const float speedLimit = std::min(std::max(maxSpeed, 0.0f), 0.8f * std::sqrt(2.0f * acceleration * std::abs(error)));
	desiredVelocity = std::clamp(desiredVelocity, -speedLimit, speedLimit);
	velocity += std::clamp(desiredVelocity - velocity, -acceleration * deltaTime, acceleration * deltaTime);
	const float step = velocity * deltaTime;
	if (step * error > 0.0f && std::abs(step) >= std::abs(error)) {
		theta += error;
		velocity = 0.0f;
	} else {
		theta += step;
	}
}
}
void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	No::Vector3 playerPos{};
	No::Vector3 playerVelocity{};
	bool playerGrounded = false;
	bool playerAllowsMovementYaw = true;
	auto playerView = registry.View<No::TransformComponent, No::VelocityComponent, No::GroundStateComponent, PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
		playerVelocity = registry.GetComponent<No::VelocityComponent>(e)->linear;
		playerGrounded = registry.GetComponent<No::GroundStateComponent>(e)->isGrounded;
		if (const auto* input = registry.GetComponent<PlayerInputComponent>(e)) {
			// 移動入力はカメラ基準。横・斜め・後ろ移動の速度を追うと、自動回転が次の移動方向を変え続ける。
			playerAllowsMovementYaw = std::abs(input->moveAxis.x) <= 0.1f && input->moveAxis.z > 0.1f;
		} else {
			playerAllowsMovementYaw = true;
		}
	}

	auto view = registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>();

	for (auto e : view) {
		if (registry.Has<CameraLockTag>(e)) {
			// 演出後に以前の判定始点や移動速度を引き継がない。
			auto* follow = registry.GetComponent<FollowCameraComponent>(e);
			follow->collisionAnchorInitialized = false;
			follow->cameraMoveVelocity = No::Vector3::ZERO;
			follow->nominalMoveVelocity = No::Vector3::ZERO;
			follow->collisionMotionActive = false;
			follow->terrainAvoidanceActive = false;
			follow->terrainAvoidanceHoldTimer = 0.0f;
			follow->terrainYawRecovering = false;
			follow->avoidanceAngularVelocity = 0.0f;
			continue;
		}
		
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);
		auto* cameraComponent = registry.GetComponent<No::CameraComponent>(e);
		const float previousFollowHeight = followCameraVariables->followHeight;
		const bool heightWasInitialized = followCameraVariables->followHeightInitialized;

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
				const float fallAlpha = 1.0f - std::exp(-std::max(followCameraVariables->heightFallFollowSpeed, 0.0f) * deltaTime);
				followCameraVariables->followHeight = No::Lerp(followCameraVariables->followHeight, playerPos.y, fallAlpha);
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
				const float fallAlpha = 1.0f - std::exp(-std::max(followCameraVariables->heightFallFollowSpeed, 0.0f) * deltaTime);
				followCameraVariables->followHeight = No::Lerp(followCameraVariables->followHeight, playerPos.y, fallAlpha);
			}
		}
		No::Vector3 cameraFollowPos = playerPos;
		cameraFollowPos.y = followCameraVariables->followHeight;
		const float followHeightVelocity = heightWasInitialized && deltaTime > 1e-6f
			? (followCameraVariables->followHeight - previousFollowHeight) / deltaTime : 0.0f;

		float hInput = No::GetInputAxisValue("CameraHorizontal");
		float vInput = No::GetInputAxisValue("CameraVertical");
		if ((std::abs(hInput) > 0.01f || std::abs(vInput) > 0.01f)
			&& followCameraVariables->manualControlTimer <= 0.0f && followCameraVariables->terrainYawRecovering) {
			// 自動回避後の手動操作は現在の向きから始め、回避前の角度へ戻さない。
			const No::Vector3 path = transform->GetWorldPosition(registry) - followCameraVariables->collisionAnchor;
			if (path.x * path.x + path.z * path.z > 1e-6f) followCameraVariables->theta = std::atan2(path.z, path.x);
			followCameraVariables->terrainYawRecovering = false;
			followCameraVariables->avoidanceAngularVelocity = 0.0f;
			followCameraVariables->terrainAvoidanceActive = false;
			followCameraVariables->terrainAvoidanceHoldTimer = 0.0f;
		}

		followCameraVariables->phi += vInput * deltaTime * followCameraVariables->rotationSpeed;
		followCameraVariables->theta += hInput * deltaTime * followCameraVariables->rotationSpeed;
		const auto angleDelta = [](float from, float to) {
			return std::remainder(to - from, 2.0f * PI);
		};
		const bool manualControl = std::abs(hInput) > 0.01f || std::abs(vInput) > 0.01f || No::InputIsPress("ViewGoal");
		if (manualControl) {
			followCameraVariables->manualControlTimer = std::max(followCameraVariables->autoFollowDelay, 0.0f);
		} else {
			followCameraVariables->manualControlTimer = std::max(0.0f, followCameraVariables->manualControlTimer - deltaTime);
		}
		const bool autoFollow = followCameraVariables->autoFollowEnabled && !manualControl
			&& followCameraVariables->manualControlTimer <= 0.0f;
		const bool terrainFollow = followCameraVariables->autoFollowEnabled && !manualControl;
		if (autoFollow && playerAllowsMovementYaw && playerVelocity.x * playerVelocity.x + playerVelocity.z * playerVelocity.z > 0.25f) {
			const float behindPlayer = std::atan2(-playerVelocity.z, -playerVelocity.x);
			const float turn = angleDelta(followCameraVariables->theta, behindPlayer);
			const float deadzone = std::max(followCameraVariables->autoFollowDeadzoneAngle, 0.0f);
			const float step = std::max(followCameraVariables->maxAutoFollowAngularSpeed, 0.0f) * deltaTime;
			if (std::abs(turn) > deadzone) {
				followCameraVariables->theta += std::copysign(std::min(step, std::abs(turn) - deadzone), turn);
			}
		}


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
		const float lookUpMinHeight = cameraFollowPos.y + std::max(followCameraVariables->lookUpCameraClearance, 0.0f);

		cameraComponent->fov = No::Lerp(followCameraVariables->minFov, followCameraVariables->maxFov, normalizedPhi);

		// 理想位置(衝突を考慮しない、zoom後のdistanceそのまま)
		No::Vector3 idealPos;
		idealPos.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		const float idealCameraY = cameraFollowPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		idealPos.y = No::Lerp(idealCameraY, std::max(idealCameraY, lookUpMinHeight), smoothLookUpBlend);
		idealPos.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		const float radius = std::max(followCameraVariables->collisionRadius, 0.001f);
		const float margin = std::max(followCameraVariables->collisionMargin, 0.0f);
		idealPos.y = std::max(idealPos.y, cameraFollowPos.y + radius + margin);
		const No::Vector3 requestedAnchor = cameraFollowPos + No::Vector3{
			0.0f, std::max(followCameraVariables->playerPosOffset, radius), 0.0f };
		// 判定始点は仮想的な注視点であり、前の足場に物理的につなぎ留めない。
		// 現在の位置で重なりだけを解消し、崖で遅れが蓄積して再配置されるのを防ぐ。
		const No::Vector3 anchorCorrection = No::MoveCapsuleTerrain(registry,
			NoEngine::CapsuleWorld{ requestedAnchor, requestedAnchor, radius + margin }, No::Vector3::ZERO);
		const No::Vector3 anchor = requestedAnchor + anchorCorrection;
		if (!followCameraVariables->collisionAnchorInitialized) {
			followCameraVariables->resolvedCameraTheta = followCameraVariables->theta;
			followCameraVariables->avoidanceAngularVelocity = 0.0f;
			followCameraVariables->terrainYawRecovering = false;
		}
		followCameraVariables->collisionAnchorInitialized = true;
		followCameraVariables->collisionAnchor = anchor;
		followCameraVariables->collisionAnchorOffset = anchorCorrection;
		idealPos += anchorCorrection;

		const auto visibleFrom = [&](const No::Vector3& start, const No::Vector3& target, float sweepRadius) {
			const No::Vector3 path = target - start;
			const float length = path.Length();
			if (length <= 1e-6f) return start;
			const auto hit = No::SweepCapsuleTerrain(registry,
				NoEngine::CapsuleWorld{ start, start, sweepRadius }, path);
			if (!hit.hit) return target;
			const float distance = std::max(0.0f, length * hit.fraction - margin);
			return start + path * (distance / length);
		};
		const auto visiblePosition = [&](const No::Vector3& target) { return visibleFrom(anchor, target, radius); };
		const No::Vector3 nowPosition = transform->GetWorldPosition(registry);
		const float horizontalRadius = followCameraVariables->distance * std::sin(followCameraVariables->phi);
		const float orbitY = idealPos.y;
		const auto orbitPosition = [&](float theta) {
			return No::Vector3{ playerPos.x + anchorCorrection.x + horizontalRadius * std::cos(theta),
				orbitY, playerPos.z + anchorCorrection.z + horizontalRadius * std::sin(theta) };
		};
		const float orbitLength = (idealPos - anchor).Length();
		const float planningRadius = radius + 0.15f;
		No::Vector3 planningAnchor = anchor;
		if (terrainFollow) {
			// 通常ジャンプで固定している高さに、架空の重力や落下を加えて左右回避させない。
			planningAnchor += No::Vector3{ playerVelocity.x, std::min(followHeightVelocity, 0.0f), playerVelocity.z } * kLookAheadTime;
			if (followHeightVelocity < 0.0f) {
				planningAnchor.y = std::max(planningAnchor.y,
					playerPos.y + std::max(followCameraVariables->playerPosOffset, radius));
			}
			planningAnchor += No::MoveCapsuleTerrain(registry,
				NoEngine::CapsuleWorld{ planningAnchor, planningAnchor, planningRadius + margin }, No::Vector3::ZERO);
		}
		const auto clearance = [&](float theta) {
			const No::Vector3 candidate = orbitPosition(theta);
			return std::min((visiblePosition(candidate) - anchor).Length(),
				(visibleFrom(planningAnchor, candidate + planningAnchor - anchor, planningRadius) - planningAnchor).Length());
		};
		const bool wasAvoiding = followCameraVariables->terrainAvoidanceActive;
		const float preferredClearance = terrainFollow ? clearance(followCameraVariables->theta) : orbitLength;
		const bool needsAvoidance = terrainFollow && orbitLength > 1e-6f && preferredClearance
			< orbitLength * std::clamp(followCameraVariables->terrainAvoidanceStartRatio, 0.0f, 1.0f);
		if (terrainFollow && (needsAvoidance || (wasAvoiding && preferredClearance < orbitLength * 0.98f))) {
			followCameraVariables->terrainAvoidanceHoldTimer = kAvoidanceHoldTime;
		} else {
			followCameraVariables->terrainAvoidanceHoldTimer = std::max(0.0f, followCameraVariables->terrainAvoidanceHoldTimer - deltaTime);
		}
		followCameraVariables->terrainAvoidanceActive = terrainFollow
			&& (needsAvoidance || followCameraVariables->terrainAvoidanceHoldTimer > 0.0f);
		if (terrainFollow && wasAvoiding && !followCameraVariables->terrainAvoidanceActive) {
			// 回避後の向きを新しい通常角度にする。古い向きへの大きな巻き戻しを発生させない。
			followCameraVariables->theta = followCameraVariables->resolvedCameraTheta;
			followCameraVariables->terrainYawRecovering = false;
			followCameraVariables->avoidanceAngularVelocity = 0.0f;
		}
		if (followCameraVariables->terrainAvoidanceActive) {
			const No::Vector3 currentPath = nowPosition - anchor;
			const float currentTheta = currentPath.x * currentPath.x + currentPath.z * currentPath.z > 1e-6f
				? std::atan2(currentPath.z, currentPath.x) : followCameraVariables->theta;
			float bestTheta = followCameraVariables->theta;
			const bool keepTarget = wasAvoiding && clearance(followCameraVariables->terrainAvoidanceTheta) >= orbitLength * 0.95f;
			if (keepTarget) {
				bestTheta = followCameraVariables->terrainAvoidanceTheta;
			} else {
				float bestScore = -1e6f;
				const auto consider = [&](float theta) {
					const float continuity = wasAvoiding ? std::abs(angleDelta(followCameraVariables->terrainAvoidanceTheta, theta)) : 0.0f;
					const float score = clearance(theta) / orbitLength
						- 0.08f * std::abs(angleDelta(followCameraVariables->theta, theta))
						- 0.18f * std::abs(angleDelta(currentTheta, theta)) - 0.15f * continuity;
					if (score > bestScore) { bestScore = score; bestTheta = theta; }
				};
				consider(followCameraVariables->theta);
				consider(currentTheta);
				if (wasAvoiding) consider(followCameraVariables->terrainAvoidanceTheta);
				for (int step = 1; step <= 6; ++step) {
					consider(followCameraVariables->theta + step * PI / 12.0f);
					consider(followCameraVariables->theta - step * PI / 12.0f);
				}
			}
			followCameraVariables->terrainAvoidanceTheta = bestTheta;
			if (!wasAvoiding && !followCameraVariables->terrainYawRecovering) {
				followCameraVariables->resolvedCameraTheta = currentTheta;
				followCameraVariables->avoidanceAngularVelocity = 0.0f;
			}
			followCameraVariables->terrainYawRecovering = true;
		}
		if (!terrainFollow) {
			followCameraVariables->resolvedCameraTheta = followCameraVariables->theta;
			followCameraVariables->avoidanceAngularVelocity = 0.0f;
			followCameraVariables->terrainYawRecovering = false;
		} else if (followCameraVariables->terrainYawRecovering) {
			const float targetTheta = followCameraVariables->terrainAvoidanceActive
				? followCameraVariables->terrainAvoidanceTheta : followCameraVariables->theta;
			AdvanceAvoidanceYaw(followCameraVariables->resolvedCameraTheta, followCameraVariables->avoidanceAngularVelocity,
				targetTheta, followCameraVariables->terrainAvoidanceAngularSpeed, deltaTime);
			if (!followCameraVariables->terrainAvoidanceActive
				&& std::abs(angleDelta(followCameraVariables->resolvedCameraTheta, targetTheta)) < 0.001f
				&& std::abs(followCameraVariables->avoidanceAngularVelocity) < 0.01f) {
				followCameraVariables->terrainYawRecovering = false;
			}
		} else {
			followCameraVariables->resolvedCameraTheta = followCameraVariables->theta;
		}
		idealPos = orbitPosition(followCameraVariables->resolvedCameraTheta);
		const No::Vector3 safeIdeal = visiblePosition(idealPos);
		const float targetDistance = (safeIdeal - anchor).Length();
		const float distanceSpeed = targetDistance < followCameraVariables->currentDistance ? 10.0f : 4.0f;
		followCameraVariables->currentDistance = No::Lerp(followCameraVariables->currentDistance, targetDistance,
			1.0f - std::exp(-distanceSpeed * deltaTime));
		const No::Vector3 idealPath = idealPos - anchor;
		const float idealLength = idealPath.Length();
		const No::Vector3 nextPosition = idealLength > 1e-6f
			? anchor + idealPath * (std::min(followCameraVariables->currentDistance, idealLength) / idealLength) : anchor;
		const float moveTime = 1.0f - std::exp(-std::max(followCameraVariables->moveSpeed, 0.0f) * deltaTime);
		// 通常の移動と入力は最初の位置補間へ戻す。カメラ全体に追加の慣性を持たせない。
		const No::Vector3 nominalPosition = No::Lerp(nowPosition, nextPosition, moveTime);
		No::Vector3 desiredPosition = nominalPosition;
		const float preservedLength = std::min((nowPosition - anchor).Length(), idealLength);
		bool needsPositionRecovery = (visiblePosition(nominalPosition) - nominalPosition).LengthSquared() > 0.01f;
		if (terrainFollow && (needsPositionRecovery || targetDistance + 0.1f < preservedLength)) {
			const No::Vector3 path = nominalPosition - anchor;
			const float recoveryLength = std::max(path.Length(), preservedLength);
			const float horizontalLength = std::sqrt(std::max(0.0f, recoveryLength * recoveryLength - path.y * path.y));
			const float theta = std::atan2(path.z, path.x);
			const auto candidateAt = [&](float offset) {
				return anchor + No::Vector3{ horizontalLength * std::cos(theta + offset), path.y,
					horizontalLength * std::sin(theta + offset) };
			};
			const auto isVisible = [&](const No::Vector3& candidate) {
				return (visiblePosition(candidate) - candidate).LengthSquared() <= 1e-6f;
			};
			const auto isReachable = [&](const No::Vector3& candidate) {
				const auto hit = No::SweepCapsuleTerrain(registry,
					NoEngine::CapsuleWorld{ nowPosition, nowPosition, radius }, candidate - nowPosition);
				return !hit.hit || hit.fraction >= 1.0f;
			};
			float bestOffset = PI;
			No::Vector3 recoveryTarget{};
			if (isVisible(candidateAt(0.0f)) && isReachable(candidateAt(0.0f))) {
				bestOffset = 0.0f;
				recoveryTarget = candidateAt(0.0f);
			}
			for (float direction : { -1.0f, 1.0f }) {
				for (int step = 1; step <= 18; ++step) {
					float upper = step * PI / 36.0f;
					if (upper - PI / 36.0f >= bestOffset) break;
					if (!isVisible(candidateAt(direction * upper))) continue;
					float lower = (step - 1) * PI / 36.0f;
					for (int iteration = 0; iteration < 8; ++iteration) {
						const float middle = (lower + upper) * 0.5f;
						if (isVisible(candidateAt(direction * middle))) upper = middle;
						else lower = middle;
					}
					const float offset = upper + 0.01f;
					const No::Vector3 candidate = candidateAt(direction * offset);
					if (!isVisible(candidate) || !isReachable(candidate)) continue;
					if (offset < bestOffset) { bestOffset = offset; recoveryTarget = candidate; }
					break;
				}
			}
			if (bestOffset < PI) {
				// 遠い安全位置へ直接移さず、そこへ向かう通常の補間を行う。
				desiredPosition = No::Lerp(nowPosition, recoveryTarget, moveTime);
				followCameraVariables->terrainAvoidanceTheta = std::atan2(recoveryTarget.z - anchor.z, recoveryTarget.x - anchor.x);
				followCameraVariables->terrainAvoidanceActive = true;
				followCameraVariables->terrainYawRecovering = true;
				followCameraVariables->terrainAvoidanceHoldTimer = kAvoidanceHoldTime;
				followCameraVariables->currentDistance = std::max(followCameraVariables->currentDistance,
					(recoveryTarget - anchor).Length());
				needsPositionRecovery = true;
			}
		}
		const bool terrainCorrection = followCameraVariables->terrainAvoidanceActive || followCameraVariables->terrainYawRecovering
			|| needsPositionRecovery || anchorCorrection.LengthSquared() > 0.0001f;
		if (!manualControl && terrainCorrection) {
			// 安全位置の探索や視線判定の結果でも、一度の更新でカメラを大きく飛ばさない。
			const float maxStep = std::max(kMaxCorrectionSpeed, playerVelocity.Length() * 1.5f) * std::max(deltaTime, 0.0f);
			desiredPosition = nowPosition + LimitLength(desiredPosition - nowPosition, maxStep);
		}
		// 位置確定後の視線投影は行わない。足場を横切る収縮がワープになるため、実体の球体移動で衝突を解決する。
		const No::Vector3 moved = No::MoveCapsuleTerrain(registry,
			NoEngine::CapsuleWorld{ nowPosition, nowPosition, radius }, desiredPosition - nowPosition);
		const No::Vector3 finalPosition = nowPosition + moved;
		transform->SetWorldPosition(registry, finalPosition);
		followCameraVariables->cameraMoveVelocity = deltaTime > 1e-6f ? moved / deltaTime : No::Vector3::ZERO;
		followCameraVariables->nominalMoveVelocity = followCameraVariables->cameraMoveVelocity;
		followCameraVariables->collisionMotionActive = terrainCorrection;
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
