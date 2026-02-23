#include "PlayerWeaponSystem.h"
#include "../Component/PlayerBulletComponent.h"
#include "../GameTag.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include <Windows.h>
#include <sstream>
#endif


// キーコード定義
#define KEY_SPACE VK_SPACE

// ============================================================
//  Update
// ============================================================

void PlayerWeaponSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);// 未使用パラメータ警告回避

	auto view = registry.View<PlayerComponent, PlayerTag, No::TransformComponent>();

	for (auto entity : view) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		HandleBulletFire(player, registry, transform->translate);

#ifdef USE_IMGUI
		DebugUI(player);
#endif
	}
}

// ============================================================
//  弾丸発射
// ============================================================

void PlayerWeaponSystem::HandleBulletFire(
	PlayerComponent* player,
	No::Registry& registry,
	const No::Vector3 playerPosition  //参照渡しにすると3.	ダングリングリファレンス発生して参照が指すメモリアドレスが無効化され、未初期化メモリを参照してたみたい？
) {
	if (!NoEngine::Input::Keyboard::IsTrigger(KEY_SPACE)) {
		return;
	}

	if (player->currentBullets <= 0) {
		return;
	}

	if (player->actualMovingDirection == Direction::None) {
		return;
	}

	auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell) {
		return;
	}
	if (!HasConnection(cell, player->actualMovingDirection)) {
		return;
	}

	player->currentBullets--;

	auto bulletEntity = registry.GenerateEntity();

	registry.AddComponent<PlayerBulletTag>(bulletEntity);
	registry.AddComponent<DeathFlag>(bulletEntity);

	auto* bullet = registry.AddComponent<PlayerBulletComponent>(bulletEntity);
	bullet->direction = DirectionToVector(player->actualMovingDirection);
	bullet->startNodeX = player->currentNodeX;
	bullet->startNodeY = player->currentNodeY;
	bullet->speed = player->bulletSpeed;
	bullet->maxDistance = 20.0f;

	auto* transform = registry.AddComponent<No::TransformComponent>(bulletEntity);
	transform->translate = playerPosition;
	transform->scale = { 0.2f, 0.2f, 0.2f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(bulletEntity);
	auto* material = registry.AddComponent<No::MaterialComponent>(bulletEntity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"PlayerBullet",
		"resources/game/td_3105/Model/ball/ball.obj",
		mesh
	);

	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("PlayerBullet");
	material->color = { 0.0f, 0.0f, 0.0f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

#ifdef USE_IMGUI
	//// デバッグ出力：弾丸発射情報
	//std::ostringstream oss;
	//oss << "[BULLET FIRE] Entity:" << bulletEntity 
	//	<< " Pos:(" << playerPosition.x << "," << playerPosition.y << "," << playerPosition.z << ")"
	//	<< " StartNode:(" << bullet->startNodeX << "," << bullet->startNodeY << ")"
	//	<< " Dir:(" << bullet->direction.x << "," << bullet->direction.y << "," << bullet->direction.z << ")\n";
	//OutputDebugStringA(oss.str().c_str());
#endif
}

No::Vector3 PlayerWeaponSystem::DirectionToVector(Direction dir) {
	switch (dir) {
	case Direction::Up:    return { 0.0f, 1.0f, 0.0f };
	case Direction::Right: return { 1.0f, 0.0f, 0.0f };
	case Direction::Down:  return { 0.0f, -1.0f, 0.0f };
	case Direction::Left:  return { -1.0f, 0.0f, 0.0f };
	default:               return { 0.0f, 0.0f, 0.0f };
	}
}

GridCellComponent* PlayerWeaponSystem::GetGridCell(
	No::Registry& registry,
	int x, int y
) {
	auto view = registry.View<GridCellComponent>();
	for (auto entity : view) {
		auto* cell = registry.GetComponent<GridCellComponent>(entity);
		if (cell->gridX == x && cell->gridY == y) {
			return cell;
		}
	}
	return nullptr;
}

bool PlayerWeaponSystem::HasConnection(
	const GridCellComponent* cell,
	Direction dir
) {
	switch (dir) {
	case Direction::Up:		return cell->hasConnectionUp;
	case Direction::Right:	return cell->hasConnectionRight;
	case Direction::Down:	return cell->hasConnectionDown;
	case Direction::Left:	return cell->hasConnectionLeft;
	default:				return false;
	}
}

// ============================================================
//  デバッグ UI
// ============================================================

#ifdef USE_IMGUI
void PlayerWeaponSystem::DebugUI(PlayerComponent* player) {
	ImGui::Begin("Player Weapon");

	ImGui::Text("自機弾関連");
	ImGui::Text("Bullets: %d / %d", player->currentBullets, player->maxBullets);

	ImGui::DragInt("最大弾丸数", &player->maxBullets, 1, 1, 99);
	ImGui::DragInt("現在の弾丸数", &player->currentBullets, 1, 0, player->maxBullets);

	if (ImGui::Button("最大値まで回復")) {
		player->currentBullets = player->maxBullets;
	}

	ImGui::DragFloat("自機弾の速度", &player->bulletSpeed, 0.1f, 1.0f, 20.0f);

	ImGui::End();
}
#endif
