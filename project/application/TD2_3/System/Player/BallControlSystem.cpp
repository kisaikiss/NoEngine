#include "BallControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/NormalEnemyComponent.h"
#include "../../Component/TrackEnemyComponent.h"
#include "../../Component/BackGroundComponent.h"

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
	
	auto backGroundView = registry.View<BackGroundComponent>();
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

				// 距離がリング半径付近で、パドルに当たる場合は反射処理
				float dist = MathCalculations::Length(ballTransform->translate);

				// 判定閾値を球の半径
				if (dist >= vausState->currentRingRadius - ballCollider->radius)
				{
					float theta = std::atan2(ballTransform->translate.y, ballTransform->translate.x);
					float diff = NormalizeAngle(theta - vausState->theta);
					constexpr float kPaddleLinearWidth = 3.5f;
					float paddleWidth = kPaddleLinearWidth * vausTransform->scale.x;
					float sinArg = std::clamp((paddleWidth * 0.5f) / vausState->currentRingRadius, -1.0f, 1.0f);
					float halfTheta = std::asin(sinArg);

					if (std::fabs(diff) <= halfTheta)
					{
						auto normal = MathCalculations::Normalize(Vector3::ZERO - ballTransform->translate);
						Vector3 tangent = MathCalculations::Normalize(Vector3(-normal.y, normal.x, 0));
						float hitFactor = diff / halfTheta;
						constexpr float kAngleBias = 0.25f;
						Vector3 dir = MathCalculations::Normalize(normal + tangent * hitFactor * kAngleBias);
						float finalSpeed = ballPhysics->baseSpeed;
						if (vausState->isReleasing)
						{
							float velocityProjected = MathCalculations::Dot(vausState->currentVelocity, dir);

							if (velocityProjected > 0.0f)
							{
								float additionalSpeed = velocityProjected * vausState->kPower;
								finalSpeed += additionalSpeed;
							}
						}
						constexpr float kMaxSpeed = 20.0f;
						finalSpeed = std::min(finalSpeed, kMaxSpeed);

						ballPhysics->velocity.x = dir.x * finalSpeed;
						ballPhysics->velocity.y = dir.y * finalSpeed;

						ballTransform->translate.x += dir.x * 0.05f;
						ballTransform->translate.y += dir.y * 0.05f;
					}

				}
			}
		}

		if (ballCollider->isCollied && (static_cast<uint32_t>(ballCollider->colliedWith) & static_cast<uint32_t>(ColliderMask::kEnemy)) != 0)
		{
			if (ballState->landed)ballState->landed = false;

			// colliedEntity が有効であることを前提とする
			if (registry.Has<No::TransformComponent>(ballCollider->colliedEntity) &&
				registry.Has<SphereColliderComponent>(ballCollider->colliedEntity))
			{
				auto* enemyTransform = registry.GetComponent<No::TransformComponent>(ballCollider->colliedEntity);
				auto* enemyCollider = registry.GetComponent<SphereColliderComponent>(ballCollider->colliedEntity);

				// XY 平面に射影した差分ベクトルを使う（Z 成分は無視）
				Vector3 diff = ballTransform->translate - enemyTransform->translate;
				diff.z = 0.0f;
				float distXY = std::sqrt(diff.x * diff.x + diff.y * diff.y);
				Vector3 normal;
				if (distXY < 1e-6f)
				{
					// 極端に近い場合は中心方向を使う
					Vector3 center2D = Vector3(ballTransform->translate.x, ballTransform->translate.y, 0.0f);
					normal = MathCalculations::Normalize(Vector3::ZERO - center2D);
				}
				else
				{
					normal = diff / distXY;
				}

				// 重なり解消（XY距離を使う）
				float penetration = (ballCollider->worldRadius + enemyCollider->worldRadius) - distXY;
				if (penetration > 0.0f)
				{
					// 小さく余分に離すことで次フレームで再び吸着するのを防止
					// Z はそのまま（レンダリング高さは保持）
					ballTransform->translate.x += normal.x * (penetration + 0.001f);
					ballTransform->translate.y += normal.y * (penetration + 0.001f);
				}

				// 相対速度が法線に向かっている（接近中）の場合のみ反射を行う
				// 法線は XY 射影なので、速度の XY 成分と比較する
				Vector3 velXY = Vector3(ballPhysics->velocity.x, ballPhysics->velocity.y, 0.0f);
				float relVelAlongNormal = MathCalculations::Dot(velXY, normal);
				if (relVelAlongNormal < 0.0f)
				{
					// 法線成分と接線成分に分けて反射・摩擦を適用（XY）
					float e = ballPhysics->coefficient; // 反発係数
					Vector3 vNormal = normal * relVelAlongNormal; // 向きは負になる
					Vector3 vTangent = velXY - vNormal;

					Vector3 vNormalAfter = -e * vNormal; // 反射（法線成分を反転して反発係数を適用）
					Vector3 vTangentAfter = vTangent * ballPhysics->friction; // 接線は摩擦で減衰

					ballPhysics->velocity.x = vNormalAfter.x + vTangentAfter.x;
					ballPhysics->velocity.y = vNormalAfter.y + vTangentAfter.y;

					// 速度が非常に小さい場合は少しだけ押し返して吸着を防止
					if (std::sqrt(ballPhysics->velocity.x * ballPhysics->velocity.x + ballPhysics->velocity.y * ballPhysics->velocity.y) < 0.05f)
					{
						ballPhysics->velocity.x += normal.x * 0.1f;
						ballPhysics->velocity.y += normal.y * 0.1f;
					}
				}
			}
		}

		// 移動更新
		ballTransform->translate.x += ballPhysics->velocity.x * deltaTime;
		ballTransform->translate.y += ballPhysics->velocity.y * deltaTime;


		Primitive::DrawSphere(ballTransform->translate, ballCollider->radius, NoEngine::Color(1.0f, 0.7f, 0.f));
	}
}