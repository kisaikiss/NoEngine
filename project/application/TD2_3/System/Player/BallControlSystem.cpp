#include "BallControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/BallStateComponent.h"

#include "../../tag.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"

using namespace NoEngine;

static float NormalizeAngle(float a)
{
	while (a > PI) a -= 2 * PI;
	while (a < -PI) a += 2 * PI;
	return a;
}

void BallControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto ballView = registry.View<
		No::TransformComponent,
		No::MaterialComponent,
		SphereColliderComponent,
		PhysicsComponent,
		BallStateComponent,
		BallTag, DeathFlag>();

	auto vausView = registry.View<
		No::TransformComponent,
		VausStateComponent>();

	for (auto entityBall : ballView)
	{
		auto* ballTransform = registry.GetComponent<No::TransformComponent>(entityBall);
		//auto* ballMaterial = registry.GetComponent<No::MaterialComponent>(entityBall);
		auto* ballCollider = registry.GetComponent<SphereColliderComponent>(entityBall);
		//auto* ballDeathFlag = registry.GetComponent<DeathFlag>(entityBall);
		auto* ballPhysics = registry.GetComponent<PhysicsComponent>(entityBall);
		auto* ballState = registry.GetComponent<BallStateComponent>(entityBall);

		for (auto entityVaus : vausView)
		{
			auto* vausTransform = registry.GetComponent<No::TransformComponent>(entityVaus);
			auto* vausState = registry.GetComponent<VausStateComponent>(entityVaus);

			if (ballState->landed)
			{
				// プラットフォームに一配置して追従させる
				Vector3 normal = MathCalculations::Normalize(-vausTransform->translate);
				ballTransform->translate =
					vausTransform->translate + normal * (ballCollider->radius * 1.75f);

				// 追従中は速度をリセット
				ballPhysics->velocity = Vector3::ZERO;
			}
			else
			{
				// not landed: 常に円の外向き重力を適用
				// 外向き方向（中心から球への単位ベクトル）
				auto outward = MathCalculations::Normalize(ballTransform->translate);
				// 加速度 = 外向き * 重力定数
				auto accel = outward * PhysicsComponent::kGravity;
				// 速度更新（useGravity フラグ適用するだけ）
				if (ballPhysics->useGravity)
				{
					ballPhysics->velocity += accel * deltaTime;
				}

				// 距離がリング半径付近で、パドルに当たる場合は反射処理（既存ロジック）
				float dist = MathCalculations::Length(ballTransform->translate);

				// 判定閾値を定数ではなく球の半径ベースに変更（球表面がリングに当たるタイミング）
				if (dist >= vausState->currentRingRadius - ballCollider->radius)
				{

					float theta = std::atan2(ballTransform->translate.y, ballTransform->translate.x);
					float diff = NormalizeAngle(theta - vausState->theta);

					float halfTheta = 3.5f / vausState->currentRingRadius * 0.5f;
					if (fabs(diff) <= halfTheta)
					{
						auto normal = MathCalculations::Normalize(Vector3::ZERO - ballTransform->translate);

						Vector3 tangent =
							MathCalculations::Normalize(
								Vector3(-normal.y, normal.x, 0)
							);

						float hitFactor = diff / halfTheta;
						constexpr float kAngleBias = 0.1f;

						Vector3 dir = MathCalculations::Normalize(
							normal + tangent * hitFactor * kAngleBias
						);

						float baseSpeed = ballPhysics->baseSpeed;

						float platformSpeed = 0.0f;
						if (vausState->hasReleasedMovement &&
							vausState->releaseTime > 0.0f)
						{
							Vector3 avgVel =
								vausState->releaseVelocityAccum / vausState->releaseTime;
							platformSpeed = MathCalculations::Length(avgVel);
						}

						float extraSpeed =
							platformSpeed + vausState->chargePower * vausState->kPower;
						extraSpeed = std::min(extraSpeed, 15.0f);

						ballPhysics->velocity =
							dir * (baseSpeed + extraSpeed);

						vausState->hasReleasedMovement = false;
						vausState->releaseTime = 0.0f;
						vausState->releaseVelocityAccum = Vector3::ZERO;
					}

				}
			}
		}

		// 敵との衝突処理（既存）
		if (ballCollider->isCollied && (static_cast<uint32_t>(ballCollider->colliedWith) & static_cast<uint32_t>(ColliderMask::kEnemy)) != 0)
		{
			// colliedEntity が有効であることを前提とする
			if (registry.Has<No::TransformComponent>(ballCollider->colliedEntity) &&
				registry.Has<SphereColliderComponent>(ballCollider->colliedEntity))
			{

				No::SoundEffectPlay("ballPong", 0.5f);
				auto* enemyTransform = registry.GetComponent<No::TransformComponent>(ballCollider->colliedEntity);
				auto* enemyCollider = registry.GetComponent<SphereColliderComponent>(ballCollider->colliedEntity);

				// 法線（球心から敵心へ）
				Vector3 diff = ballTransform->translate - enemyTransform->translate;
				float dist = MathCalculations::Length(diff);
				Vector3 normal;
				if (dist < 1e-6f)
				{
					// 極端に近い場合は中心方向を使う
					normal = MathCalculations::Normalize(ballTransform->translate);
				}
				else
				{
					normal = diff / dist;
				}

				// 重なり解消
				float penetration = (ballCollider->worldRadius + enemyCollider->worldRadius) - dist;
				if (penetration > 0.0f)
				{
					// 小さく余分に離すことで次フレームで再び吸着するのを防止
					ballTransform->translate += normal * (penetration + 0.001f);
				}

				// 相対速度が法線に向かっている（接近中）の場合のみ反射を行う
				float relVelAlongNormal = MathCalculations::Dot(ballPhysics->velocity, normal);
				if (relVelAlongNormal < 0.0f)
				{
					// 法線成分と接線成分に分けて反射・摩擦を適用
					float e = ballPhysics->coefficient; // 反発係数
					Vector3 vNormal = normal * relVelAlongNormal; // 向きは負になる
					Vector3 vTangent = ballPhysics->velocity - vNormal;

					Vector3 vNormalAfter = -e * vNormal; // 反射（法線成分を反転して反発係数を適用）
					Vector3 vTangentAfter = vTangent * ballPhysics->friction; // 接線は摩擦で減衰

					ballPhysics->velocity = vNormalAfter + vTangentAfter;

					// 速度が非常に小さい場合は少しだけ押し返して吸着を防止
					if (MathCalculations::Length(ballPhysics->velocity) < 0.05f)
					{
						ballPhysics->velocity += normal * 0.1f;
					}
				}
			}		}

		// 移動更新
		ballTransform->translate += ballPhysics->velocity * deltaTime;


		Primitive::DrawSphere(ballTransform->translate, ballCollider->radius, NoEngine::Color(1.0f, 0.7f, 0.f));
	}
}