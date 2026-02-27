#include "PlayerWeaponSystem.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/ColliderComponent.h"
#include "../GameTag.h"
#include "../Utility/GridUtils.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include <Windows.h>
#include <sstream>
#endif

#define KEY_SPACE VK_SPACE

// ============================================================
//  Update
// ============================================================

void PlayerWeaponSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto view = registry.View<PlayerComponent, PlayerTag, No::TransformComponent>();
	if (view.Empty())return;

	for (auto entity : view) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		HandleBulletFire(player, registry, transform->translate, transform->rotation);

#ifdef USE_IMGUI
		DebugUI(player);
#endif
	}
}

// ============================================================
//  HandleBulletFire
// ============================================================

void PlayerWeaponSystem::HandleBulletFire(
	PlayerComponent* player,
	No::Registry& registry,
	const No::Vector3 playerPosition,      // 値渡し：ダングリングリファレンス防止
	const NoEngine::Math::Quaternion playerRotation  // 値渡し：ダングリングリファレンス防止
) {
	if (!NoEngine::Input::Keyboard::IsTrigger(KEY_SPACE)) return;
	if (player->currentBullets <= 0)                       return;
	if (player->actualMovingDirection == Direction::None)  return;

	auto* cell = GridUtils::GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell)                                                                  return;
	if (!GridUtils::HasConnection(cell, player->actualMovingDirection))         return;

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

	// SphereCollider 追加
	auto* collider = registry.AddComponent<SphereColliderComponent>(bulletEntity);
	collider->radius = 0.5f;                // モデル空間での半径（直径1の球体の半径）
	collider->colliderType = kPlayerBullet; // プレイヤーの弾タイプ
	collider->collideMask = kEnemy;         // 敵とのみ衝突

	auto* transform = registry.AddComponent<No::TransformComponent>(bulletEntity);
	transform->translate = playerPosition;
	transform->scale = { 0.2f, 0.2f, 0.2f };
	// 回転はplayerの向きと同じにする
	transform->rotation = playerRotation;

	auto* mesh = registry.AddComponent<No::MeshComponent>(bulletEntity);
	auto* material = registry.AddComponent<No::MaterialComponent>(bulletEntity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"PlayerBullet",
		"resources/game/td_3105/Model/Shot/Shot.obj",
		mesh
	);

	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("PlayerBullet");
	material->color = { 0.0f, 0.0f, 0.0f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

// ============================================================
//  DirectionToVector
// ============================================================

No::Vector3 PlayerWeaponSystem::DirectionToVector(Direction dir) {
	switch (dir) {
	case Direction::Up:    return { 0.0f,  1.0f, 0.0f };
	case Direction::Right: return { 1.0f,  0.0f, 0.0f };
	case Direction::Down:  return { 0.0f, -1.0f, 0.0f };
	case Direction::Left:  return { -1.0f, 0.0f, 0.0f };
	default:               return { 0.0f,  0.0f, 0.0f };
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