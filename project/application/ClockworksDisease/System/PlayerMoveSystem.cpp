#include "PlayerMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

using namespace std;

void PlayerMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent>();
	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		playerVariables->velocity = No::Vector3::ZERO;

		// キーボード入力による移動を行います
		if (No::Keyboard::IsPress('D')) {
			playerVariables->velocity.x += 1;
		}
		if (No::Keyboard::IsPress('A')) {
			playerVariables->velocity.x -= 1;
		}
		if (No::Keyboard::IsPress('W')) {
			playerVariables->velocity.z += 1;
		}
		if (No::Keyboard::IsPress('S')) {
			playerVariables->velocity.z -= 1;
		}

		// 入力がされていた場合
		if (playerVariables->velocity.x || playerVariables->velocity.z) {
			// 移動成分を正規化
			playerVariables->velocity = playerVariables->velocity.Normalize() * deltaTime;

			// 実際に移動する
			transform->translate += playerVariables->velocity * playerVariables->moveSpeed ;

			// プレイヤーを移動方向へ向ける
			transform->rotation.LookRotation(playerVariables->velocity, No::Vector3::UP);

		}


		transform->translate += playerVariables->velocity;
	}
}
