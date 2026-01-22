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
		BallTag,DeathFlag>();

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
				// プラットフォームに一貫して追従させる
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

				// 距離がリング半径近傍で、パドルに当たる場合は反射処理（既存ロジック）
				float dist = MathCalculations::Length(ballTransform->translate);
				if (dist >= vausState->currentRingRadius - 0.25f)
				{
					float theta = std::atan2(ballTransform->translate.y, ballTransform->translate.x);
					float diff = NormalizeAngle(theta - vausState->theta);

					float halfTheta = 3.5f / vausState->currentRingRadius * 0.5f;
					if (std::fabs(diff) <= halfTheta)
					{
						constexpr float tangentStrength = 0.8f;
						// 法線方向（円の中心から球に向かう方向）
						auto normal = MathCalculations::Normalize(ballTransform->translate);
						float targetDist = vausState->currentRingRadius - ballCollider->radius;
						ballTransform->translate = normal * targetDist;

						// 反射
						float dot = MathCalculations::Dot(ballPhysics->velocity, normal);
						auto reflected = ballPhysics->velocity - 2 * dot * normal;
						reflected *= ballPhysics->coefficient;
						// 接線方向（円弧方向）を計算します。
						auto tangent = MathCalculations::Normalize(Vector3(-normal.y, normal.x, 0.0f));
						//接触点はバウンス角に影響します: -1 ~ 1
						float hitFactor = diff / halfTheta;
						// 接線オフセットを追加
						reflected += tangent * hitFactor * tangentStrength;

						//速度更新（速度大きさは維持）
						float speed = MathCalculations::Length(ballPhysics->velocity);
						reflected = MathCalculations::Normalize(reflected) * speed;
						float impulse =
							vausState->chargePower * VausStateComponent::kPower;
						reflected += normal * impulse;

						ballPhysics->velocity = reflected;
					}
				}
			}
		}

		if (ballCollider->isCollied && (ballCollider->colliderType & ColliderMask::kEnemy) == 0)
		{
			auto* enemyTransform = registry.GetComponent<No::TransformComponent>(ballCollider->colliedEntity);
			auto normal = MathCalculations::Normalize(ballTransform->translate - enemyTransform->translate);
			float dot = MathCalculations::Dot(ballPhysics->velocity, normal);
			auto reflected = ballPhysics->velocity - 2 * dot * normal;
			float speed = MathCalculations::Length(ballPhysics->velocity);
			reflected = MathCalculations::Normalize(reflected) * speed;
			reflected *= ballPhysics->coefficient;

			ballPhysics->velocity = reflected;
		}

		// 移動更新
		ballTransform->translate += ballPhysics->velocity * deltaTime;


		Primitive::DrawSphere(ballTransform->translate, ballCollider->radius, NoEngine::Color(1.0f,0.7f,0.f));
	}
}

