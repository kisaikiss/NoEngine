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

		cameraComponent->fov = No::Lerp(followCameraVariables->minFov, followCameraVariables->maxFov, normalizedPhi);

		// 理想位置(衝突を考慮しない、zoom後のdistanceそのまま)
		No::Vector3 idealPos;
		idealPos.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		idealPos.y = cameraFollowPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		idealPos.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		// playerPos -> idealPos の経路上を地形と判定し、詰めるべき距離を求める
		float targetDistance = followCameraVariables->distance;

		// 判定する範囲をカメラからプレイヤーまでではなく、プレイヤーから少しカメラに近い位置にする
		No::Vector3 toCamera = idealPos - cameraFollowPos;
		float toCameraLength = toCamera.Length();

		No::Vector3 sweepStart = cameraFollowPos;
		if (toCameraLength > 1e-4f) {
			// distance未満にclampしてから始点をずらす(始点が終点を追い越さないように)
			float offset = std::min(followCameraVariables->collisionStartOffset, toCameraLength);
			sweepStart = cameraFollowPos + (toCamera / toCameraLength) * offset;
		}

		auto terrainView = registry.View<No::TerrainMesh>();
		for (auto terrainE : terrainView) {
			auto* terrain = registry.GetComponent<No::TerrainMesh>(terrainE);

			// 線分(半径込み)を包むAABBでBVHに問い合わせ、候補三角形を絞り込む
			No::AABBCollider sweepBounds;
			sweepBounds.min = No::Vector3{
				std::min(sweepStart.x, idealPos.x) - followCameraVariables->collisionRadius,
				std::min(sweepStart.y, idealPos.y) - followCameraVariables->collisionRadius,
				std::min(sweepStart.z, idealPos.z) - followCameraVariables->collisionRadius
			};
			sweepBounds.max = No::Vector3{
				std::max(sweepStart.x, idealPos.x) + followCameraVariables->collisionRadius,
				std::max(sweepStart.y, idealPos.y) + followCameraVariables->collisionRadius,
				std::max(sweepStart.z, idealPos.z) + followCameraVariables->collisionRadius
			};

			candidateIndices_.clear();
			No::QueryBVH(terrain->bvhRoot.get(), sweepBounds, terrain->triangles, candidateIndices_);

			for (int idx : candidateIndices_) {
				const auto& triangle = terrain->triangles[idx];
				auto collide = No::TestSegmentTriangle(sweepStart, idealPos, followCameraVariables->collisionRadius, triangle);
				if (!collide.hit) continue;

				// playerPosから衝突点までの距離を「その方向で許容できる距離」とみなす
				float hitDistance = (collide.closestOnCapsule - cameraFollowPos).Length();
				targetDistance = std::min(targetDistance, std::max(hitDistance - followCameraVariables->collisionMargin, 0.5f));
			}
		}

		// 詰めるときは速く、戻すときはゆっくり
		float smoothSpeed = (targetDistance < followCameraVariables->currentDistance) ? 25.0f : 4.0f;
		followCameraVariables->currentDistance = No::Lerp(
			followCameraVariables->currentDistance, targetDistance,
			1.0f - std::exp(-smoothSpeed * deltaTime));

		// 移動する先の座標計算
		No::Vector3 nextPosition = No::Vector3::ZERO;
		nextPosition.x = cameraFollowPos.x + followCameraVariables->currentDistance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		nextPosition.y = cameraFollowPos.y + followCameraVariables->currentDistance * std::cos(followCameraVariables->phi);
		nextPosition.z = cameraFollowPos.z + followCameraVariables->currentDistance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);


		// 実際の移動
		// 追従速度がフレームレートに依存しないようにする。
		const float moveTime = 1.0f - std::exp(-followCameraVariables->moveSpeed * deltaTime);
		No::Vector3 nowTranslate = transform->translate;
		transform->translate = No::Lerp(nowTranslate, nextPosition, moveTime);

		// カメラの実座標での衝突判定
		for (int iter = 0; iter < 2; ++iter) {
			No::Vector3 camPos = transform->GetWorldPosition(registry);

			No::AABBCollider probeBounds;
			probeBounds.min = camPos - No::Vector3{ followCameraVariables->collisionRadius, followCameraVariables->collisionRadius, followCameraVariables->collisionRadius };
			probeBounds.max = camPos + No::Vector3{ followCameraVariables->collisionRadius, followCameraVariables->collisionRadius, followCameraVariables->collisionRadius };

			bool anyHit = false;
			for (auto terrainE : terrainView) {
				auto* terrain = registry.GetComponent<No::TerrainMesh>(terrainE);

				candidateIndices_.clear();
				No::QueryBVH(terrain->bvhRoot.get(), probeBounds, terrain->triangles, candidateIndices_);

				for (int idx : candidateIndices_) {
					const auto& triangle = terrain->triangles[idx];
					auto push = No::TestSphereTriangle(camPos, followCameraVariables->collisionRadius, triangle);
					if (!push.hit) continue;

					// めり込み分だけ即座に(Lerpなしで)押し戻す
					transform->translate += push.normal * push.penetration;
					camPos += push.normal * push.penetration;
					anyHit = true;
				}
			}
			if (!anyHit) break;
		}
		

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
