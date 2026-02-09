#include "BallControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/BallTrailComponent.h"
#include "../../Component/PlayerstatusComponent.h"

#include "../../tag.h"
#include "engine/Math/Easing.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Assets/ModelLoader.h"
#include "externals/imgui/imgui.h"

using namespace No;
using namespace NoEngine;
static float NormalizeAngle(float a)
{
	while (a > PI) a -= 2 * PI;
	while (a < -PI) a += 2 * PI;
	return a;
}

void BallControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView)
	{
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade)
		{
			// レベルアップ選択中は操作を受け付けない
			return;
		}
	}

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

	// まず Vaus の参照を取得（スポーン/閾値チェックで使うため）
	No::TransformComponent* vausTransformPtr = nullptr;
	VausStateComponent* vausStatePtr = nullptr;
	for (auto e : vausView)
	{
		vausTransformPtr = registry.GetComponent<No::TransformComponent>(e);
		vausStatePtr = registry.GetComponent<VausStateComponent>(e);
	}

	ballCount_ = 0;
	for (auto entityBall : ballView)
	{
		auto* ballTransform = registry.GetComponent<No::TransformComponent>(entityBall);
		//auto* ballMaterial = registry.GetComponent<No::MaterialComponent>(entityBall);
		auto* ballCollider = registry.GetComponent<SphereColliderComponent>(entityBall);
		//auto* ballDeathFlag = registry.GetComponent<DeathFlag>(entityBall);
		auto* ballPhysics = registry.GetComponent<PhysicsComponent>(entityBall);
		auto* ballState = registry.GetComponent<BallStateComponent>(entityBall);
		BallTrailComponent* trail = nullptr;
		if (registry.Has<BallTrailComponent>(entityBall))
		{
			trail = registry.GetComponent<BallTrailComponent>(entityBall);
		}
		auto* deathFlag = registry.GetComponent<DeathFlag>(entityBall);

		if (vausStatePtr)
		{
			float dist = MathCalculations::Length(ballTransform->translate);
			const float kDestroyOuterFactor = 2.0f;
			if (dist > vausStatePtr->currentRingRadius * kDestroyOuterFactor)
			{
				if (deathFlag) deathFlag->isDead = true;
				// 削除マークを付けたのでこのフレームはそれ以上処理しない
				continue;
			}
		}

		// この時点で生存している（削除マークのついていない）ボールのみをカウント
		ballCount_++;

#ifndef RELEASE
		if (Input::Keyboard::IsTrigger('R'))
		{
			ballState->landed = true;
			ballState->isOut = false;
		}
		if (trail)
		{
			ImGui::Begin("Ball Trail");
			ImGui::ColorEdit4("startColor", &trail->startColor.r);
			ImGui::ColorEdit4("endColor", &trail->endColor.r);
			ImGui::End();
		}

#endif // !RELEASE

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
				playerStatus->scoreRatio = 1.0f;
				playerStatus->isComboing = false;
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
				if (!ballState->isOut && dist >= vausState->currentRingRadius - ballCollider->radius)
				{
					/*No::SoundPlay("ballPong", 0.5f,false);*/

					float theta = std::atan2(ballTransform->translate.y, ballTransform->translate.x);
					float diff = NormalizeAngle(theta - vausState->theta);

					float paddleWidth = vausState->widthScale * vausState->kBaseWidth * vausTransform->scale.x;
					float sinArg = std::clamp((paddleWidth * 0.5f) / vausState->currentRingRadius, -1.0f, 1.0f);
					float halfTheta = std::asin(sinArg);

					if (std::fabs(diff) <= halfTheta)
					{
						auto normal = MathCalculations::Normalize(Vector3::ZERO - ballTransform->translate);
						Vector3 tangent = MathCalculations::Normalize(Vector3(-normal.y, normal.x, 0));
						float hitFactor = -diff / halfTheta;
						constexpr float kAngleBias = 0.25f;
						Vector3 dir = MathCalculations::Normalize(normal + tangent * hitFactor * kAngleBias);

						// --- ここからランダム性の付与（軽いジッターで反射角を固定化しすぎない） ---
						// 小さめの最大ジッター（ラジアン）。値は調整可能。
						constexpr float kReflectionJitter = 0.12f;
						// hitFactor が端に近いほどジッターを少し抑える（好みで変更可）
						float jitterScale = 1.0f - 0.5f * std::fabs(hitFactor);
						float raw = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); // 0..1
						float jitter = (raw - 0.5f) * 2.0f * kReflectionJitter * jitterScale;

						// 2D 回転で dir をジッター回転させる
						float cosA = std::cos(jitter);
						float sinA = std::sin(jitter);
						Vector3 dir2{
							dir.x * cosA - dir.y * sinA,
							dir.x * sinA + dir.y * cosA,
							0.0f
						};
						dir = MathCalculations::Normalize(dir2);
						// --- ランダム性付与ここまで ---

						float finalSpeed = ballPhysics->baseSpeed;
						if (vausState->isReleasing)
						{
							float velocityProjected = MathCalculations::Dot(vausState->currentVelocity, dir);

							if (velocityProjected > 0.0f)
							{
								float additionalSpeed = velocityProjected * vausState->chargePower;
								finalSpeed += additionalSpeed;
							}
						}
						constexpr float kMaxSpeed = 20.0f;
						finalSpeed = std::min(finalSpeed, kMaxSpeed);

						ballPhysics->velocity.x = dir.x * finalSpeed;
						ballPhysics->velocity.y = dir.y * finalSpeed;

						ballTransform->translate.x += dir.x * 0.05f;
						ballTransform->translate.y += dir.y * 0.05f;
						ballState->isOut = false;
						playerStatus->scoreRatio = 1.0f;
						playerStatus->isComboing = false;

						const std::string soundName[] = { 
							"ballPong", "ballPong2"};

						int soundIndex = std::rand() % 2;
						No::SoundEffectPlay(soundName[soundIndex], 0.5f);
					}
					else
					{
						ballState->isOut = true;
					}
				}
			}
		}

		if (ballCollider->isCollied && (static_cast<uint32_t>(ballCollider->colliedWith) & static_cast<uint32_t>(ColliderMask::kEnemy)) != 0)
		{
			if (ballState->landed)ballState->landed = false;

			if (playerStatus)
			{
				if (!playerStatus->isComboing)playerStatus->isComboing = true;
				else
				{
					playerStatus->scoreRatio += 0.25f;
				}
			}
			// colliedEntity が有効であることを前提とする
			if (registry.Has<No::TransformComponent>(ballCollider->colliedEntity) &&
				registry.Has<SphereColliderComponent>(ballCollider->colliedEntity))
			{

				No::SoundEffectPlay("ballPong", 0.5f);
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
		ballTransform->translate.x += ballPhysics->velocity.x * deltaTime * ballState->ballSlowFactor;
		ballTransform->translate.y += ballPhysics->velocity.y * deltaTime * ballState->ballSlowFactor;
		if (trail)
		{
			if (deltaTime > 0.0f)
				trail->sampleInterval = deltaTime;
			// 既存サンプルを経過時間分だけ進める
			for (auto& s : trail->samples)s.age += deltaTime;

			// サンプリングタイミング管理
			trail->timeSinceLast += deltaTime;
			if (trail->timeSinceLast >= trail->sampleInterval)
			{
				trail->timeSinceLast = 0.0f;
				BallTrailComponent::Sample sample;
				sample.pos = ballTransform->translate;
				sample.age = 0.0f;
				// newest を front に入れる
				trail->samples.push_front(sample);
				if (trail->samples.size() > trail->maxSamples)
				{
					trail->samples.pop_back();
				}
			}

			// 期限切れサンプルを削除
			while (!trail->samples.empty() && trail->samples.back().age > trail->maxAge)
			{
				trail->samples.pop_back();
			}
		}
	}

	if (ballCount_ == 0)
	{
		if (vausStatePtr)
			vausStatePtr->isHerted = true;
		// プレイヤー HP を -1 する

		if (playerStatus)
		{
			playerStatus->hp -= 1;
			for (int i = 0; i < playerStatus->ballCount; i++)
			{
				// 新しいボールを一つ生成（Vaus 位置があればそこを基準に、無ければ原点付近）
				No::Entity ballEntity = registry.GenerateEntity();
				registry.AddComponent<BallTag>(ballEntity);
				registry.AddComponent<PhysicsComponent>(ballEntity);
				registry.AddComponent<BallStateComponent>(ballEntity);

				auto* trailComp = registry.AddComponent<BallTrailComponent>(ballEntity);
				trailComp->maxAge = 0.6f;
				trailComp->sampleInterval = 0.02f;
				trailComp->thickness = 0.35f;
				trailComp->maxSamples = 256;

				auto* collider = registry.AddComponent<SphereColliderComponent>(ballEntity);
				collider->radius = 0.25f;
				collider->colliderType = ColliderMask::kBall;
				collider->collideMask = ColliderMask::kEnemy;

				registry.AddComponent<DeathFlag>(ballEntity);

				auto* transform = registry.AddComponent<No::TransformComponent>(ballEntity);
				// スポーン位置: Vaus の外側少し内側に配置する（参照が無ければ既存のデフォルト）
				if (vausTransformPtr && vausStatePtr)
				{
					Vector3 normal = MathCalculations::Normalize(-vausTransformPtr->translate);
					transform->translate = vausTransformPtr->translate + normal * (collider->radius * 1.75f);
				}
				else
				{
					transform->translate = { 0.0f, -4.35f, 0.f };
				}

				auto* model = registry.AddComponent<No::MeshComponent>(ballEntity);
				NoEngine::ModelLoader::LoadModel("ball", "resources/game/td_2304/Model/ball/ball.obj", model);

				auto m = registry.AddComponent<No::MaterialComponent>(ballEntity);
				m->materials = NoEngine::ModelLoader::GetMaterial("ball");

				m->psoName = L"Renderer : Default PSO";
				m->psoId = NoEngine::Render::GetPSOID(m->psoName);
				m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
			}
		}
	}

	if (vausStatePtr->isHerted)
	{
		if (vausStatePtr->playerDamageCooldown < vausStatePtr->kDamageCooldownTime)
			vausStatePtr->playerDamageCooldown += deltaTime;
		else
		{
			vausStatePtr->isHerted = false;
			vausStatePtr->playerDamageCooldown = 0.0f;
		}
		auto vausMeshView = registry.View<No::MeshComponent, VausTag>();
		if (std::fmod(vausStatePtr->playerDamageCooldown, 0.2f) < 0.1f)
		{
			for (auto e : vausMeshView)
			{
				auto* vausMesh = registry.GetComponent<No::MeshComponent>(e);
				vausMesh->isVisible = true;
			}
		}
		else
		{
			for (auto e : vausMeshView)
			{
				auto* vausMesh = registry.GetComponent<No::MeshComponent>(e);
				vausMesh->isVisible = false;
			}
		}
	}

	if (BallStateComponent::ballSlowtime < BallStateComponent::kSlowDuration)
	{
		BallStateComponent::ballSlowtime += deltaTime;
		float t = BallStateComponent::ballSlowtime / BallStateComponent::kSlowDuration;
		BallStateComponent::ballSlowFactor = Easing::EaseInExpo(
			BallStateComponent::kSlowRcp, 1.0f, t);
	}
}