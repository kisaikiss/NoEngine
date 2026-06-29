#include "PlayerPushBackSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Player/PlayerComponent.h"

void PlayerPushBackSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto playerView = registry.View<PlayerComponent, No::GroundStateComponent>();

	auto view = registry.PollAllEvents<PlayerPushBackEvent>();
	for (const auto& event : view) {
		if (event.position == No::ContactPosition::UP) {
			auto* transform = registry.GetComponent<No::TransformComponent>(event.player);
			auto* ground = registry.GetComponent<No::GroundStateComponent>(event.player);
			
			ground->isGrounded = true;
			ground->groundHeight = transform->GetWorldPosition().y;


			auto* player = registry.GetComponent<PlayerComponent>(event.player);


			if (player->yVelocity < 0.f) {
				if (!ground->preIsGrounded && player->yVelocity < -3.5f) {
					registry.AddComponent<No::EffectEmitTag>(event.player);
				}
				player->yVelocity = 0.f;
			}
			if (event.normal.y > player->groundNormal.y) {
				player->groundNormal = event.normal;
			}
			No::Vector3 currentForward = transform->rotation.zAxis();
			// 前方向を groundNormal 平面へ投影して再正規化（体が地面から浮かない）
			float dot = currentForward.Dot(event.normal);
			No::Vector3 projForward = currentForward - event.normal * dot;
			float len = projForward.Length();
			if (len > 1e-6f) {
				projForward = projForward * (1.f / len);

				No::Quaternion newRotation;
				newRotation.LookRotation(projForward, event.normal);
				const float kSlerpScale = 20.f;
				transform->rotation = transform->rotation.Slerp(transform->rotation, newRotation, deltaTime * kSlerpScale);
			}
		}
		if (event.position == No::ContactPosition::DOWN) {
			auto* player = registry.GetComponent<PlayerComponent>(event.player);
			if (player->yVelocity > 0.f) {
				player->yVelocity = 0.f;
			}

		}


	}

	for (auto entity : playerView) {
		// 空中では groundNormal を UP に戻す
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* ground = registry.GetComponent<No::GroundStateComponent>(entity);
		if (!ground->isGrounded) {
			player->groundNormal = No::Vector3::UP;

		} 
	}
}
