#include "EnemySpawnerSystem.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../System/GameTimer.h"
#include "../GameTag.h"
#include "../Utility/GridUtils.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// ============================================================
//  Update
// ============================================================

void EnemySpawnerSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;

	if (!gameTimer_) return;

	// spawnInterval_ が変更されたとき全スポナーの速度を再計算する
	if (spawnInterval_ != prevSpawnInterval_) {
		RecalculateAllSpeeds(registry);
		prevSpawnInterval_ = spawnInterval_;
	}

	float gameDeltaTime = gameTimer_->GetGameDeltaTime();

#ifdef USE_IMGUI
	DebugUI(registry);
#endif

	if (gameDeltaTime <= 0.0f) return;

	auto view = registry.View<EnemySpawnerComponent, EnemySpawnerTag>();
	for (auto entity : view) {
		auto* spawner = registry.GetComponent<EnemySpawnerComponent>(entity);

		spawner->spawnTimer += gameDeltaTime;

		if (spawner->spawnTimer >= spawnInterval_) {
			spawner->spawnTimer -= spawnInterval_;
			SpawnEnemy(registry, spawner);
		}
	}
}

// ============================================================
//  SetupSpawners
//  ステージロード後に SampleScene から呼ぶ。
//  各スポナーの spawnDirection を自動検出し、
//  chainCount（敵専用道の連なり数）と calculatedSpeed を計算する。
// ============================================================

void EnemySpawnerSystem::SetupSpawners(No::Registry& registry) {
	const Direction allDirs[4] = {
		Direction::Up, Direction::Right, Direction::Down, Direction::Left
	};

	auto view = registry.View<EnemySpawnerComponent, EnemySpawnerTag>();
	for (auto entity : view) {
		auto* spawner = registry.GetComponent<EnemySpawnerComponent>(entity);

		// スポーンタイマーをリセット（ステージ再ロード対応）
		spawner->spawnTimer = 0.0f;

		// ---- spawnDirection を自動検出 ----
		// スポナーノードの接続先のうち、最初に見つかった有効なノードへの方向を使う。
		// スポナーノード自体は isEnemyOnly=true のため、接続先も isEnemyOnly が期待される。
		Direction spawnDir = Direction::None;
		{
			auto* cell = GridUtils::GetGridCell(registry, spawner->nodeX, spawner->nodeY);
			if (cell) {
				for (Direction dir : allDirs) {
					if (!GridUtils::HasConnection(cell, dir)) continue;
					int nx, ny;
					GridUtils::GetNextNodeCoords(spawner->nodeX, spawner->nodeY, dir, nx, ny);
					auto* nextCell = GridUtils::GetGridCell(registry, nx, ny);
					if (nextCell) {
						spawnDir = dir;
						break;
					}
				}
			}
		}
		spawner->spawnDirection = spawnDir;

		if (spawnDir == Direction::None) {
			spawner->chainCount      = 0;
			spawner->calculatedSpeed = 1.0f;
			continue;
		}

		// ---- chainCount を計算 ----
		// スポナーから spawnDirection に向かって、
		// 通常ノード（isEnemyOnly=false）に到達するまでの エッジ数 を数える。
		// 1 エッジ = 1 グリッド単位。
		int count = 0;
		int cx = spawner->nodeX;
		int cy = spawner->nodeY;
		Direction curDir = spawnDir;

		while (curDir != Direction::None) {
			int nx, ny;
			GridUtils::GetNextNodeCoords(cx, cy, curDir, nx, ny);
			auto* nextCell = GridUtils::GetGridCell(registry, nx, ny);
			if (!nextCell) break;

			count++;

			// 通常ノードに到達したらカウント終了
			if (!nextCell->isEnemyOnly) break;

			// 次のノードへ進む（来た方向の逆以外で次の接続を探す）
			Direction back    = GridUtils::GetOppositeDirection(curDir);
			Direction nextDir = Direction::None;
			for (Direction d : allDirs) {
				if (d == back) continue;
				if (GridUtils::HasConnection(nextCell, d)) {
					nextDir = d;
					break;
				}
			}
			cx     = nx;
			cy     = ny;
			curDir = nextDir;
		}

		spawner->chainCount = count;

		// speed = chainCount / spawnInterval
		// → ちょうど spawnInterval 秒で通常道に到達する速度
		spawner->calculatedSpeed = (count > 0 && spawnInterval_ > 0.0f)
			? static_cast<float>(count) / spawnInterval_
			: 1.0f;
	}

	prevSpawnInterval_ = spawnInterval_;
}

// ============================================================
//  RecalculateAllSpeeds
//  spawnInterval_ が変わったとき全スポナーの calculatedSpeed を更新する
// ============================================================

void EnemySpawnerSystem::RecalculateAllSpeeds(No::Registry& registry) {
	auto view = registry.View<EnemySpawnerComponent, EnemySpawnerTag>();
	for (auto entity : view) {
		auto* spawner = registry.GetComponent<EnemySpawnerComponent>(entity);
		spawner->calculatedSpeed = (spawner->chainCount > 0 && spawnInterval_ > 0.0f)
			? static_cast<float>(spawner->chainCount) / spawnInterval_
			: 1.0f;
	}
}

// ============================================================
//  SpawnEnemy
//  スポナー位置に敵を 1 体生成する。
//  生成した敵は isSpawning=true で初期化され、
//  lastDirection にスポーン方向の逆を設定することで後退を防ぐ。
// ============================================================

void EnemySpawnerSystem::SpawnEnemy(No::Registry& registry, EnemySpawnerComponent* spawner) {
	auto entity = registry.GenerateEntity();

	auto* enemy = registry.AddComponent<EnemyComponent>(entity);
	enemy->currentNodeX = spawner->nodeX;
	enemy->currentNodeY = spawner->nodeY;
	enemy->targetNodeX  = spawner->nodeX;
	enemy->targetNodeY  = spawner->nodeY;
	enemy->state        = PlayerState::OnNode;

	// 後退防止: 来た方向（=スポーン方向の逆）を lastDirection に設定する
	enemy->lastDirection = GridUtils::GetOppositeDirection(spawner->spawnDirection);

	// スポーニング状態の初期化
	enemy->isSpawning     = true;
	enemy->spawningSpeed  = spawner->calculatedSpeed;
	enemy->spawnExitTimer = 0.0f;

	registry.AddComponent<EnemyTag>(entity);
	registry.AddComponent<DeathFlag>(entity);
	registry.AddComponent<HealthComponent>(entity);

	// コライダー（スポーニング状態は青）
	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->radius       = 0.5f;
	collider->colliderType = kEnemy;
	collider->collideMask  = kPlayer | kPlayerBullet | kEnemy | kShockwave;
	collider->debugColor   = { 0.0f, 0.0f, 1.0f }; // 青: スポーニング中

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->translate = GridUtils::GridToWorld(spawner->nodeX, spawner->nodeY);
	transform->scale     = { 0.2f, 0.2f, 0.2f };

	auto* mesh     = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"Enemy",
		"resources/game/td_3105/Model/Enemy/Enemy.obj",
		mesh
	);
	material->materials  = NoEngine::Asset::ModelLoader::GetMaterial("Enemy");
	material->color      = { 0.0f, 0.5f, 1.0f, 1.0f }; // 青系: スポーニング中
	material->psoName    = L"Renderer : Default PSO";
	material->psoId      = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId  = NoEngine::Render::GetRootSignatureID(material->psoName);
}

// ============================================================
//  デバッグ UI
// ============================================================

#ifdef USE_IMGUI
void EnemySpawnerSystem::DebugUI(No::Registry& registry) {
	ImGui::Begin("Enemy Spawner");

	// ---- スポーン間隔（全スポナー共通）----
	ImGui::Text("スポーン設定（全スポナー共通）");
	ImGui::DragFloat("スポーン間隔(秒)", &spawnInterval_, 0.1f, 0.5f, 30.0f, "%.1f");
	if (spawnInterval_ < 0.5f) spawnInterval_ = 0.5f;

	ImGui::Separator();

	// ---- 各スポナーの状態 ----
	auto view = registry.View<EnemySpawnerComponent, EnemySpawnerTag>();
	int idx = 0;

	for (auto entity : view) {
		auto* spawner = registry.GetComponent<EnemySpawnerComponent>(entity);

		ImGui::Text("Spawner [%d] @ (%d, %d)", idx++, spawner->nodeX, spawner->nodeY);

		// 残り時間ゲージ
		float progress = (spawnInterval_ > 0.0f)
			? spawner->spawnTimer / spawnInterval_
			: 0.0f;
		char overlay[32];
		snprintf(overlay, sizeof(overlay), "%.1f / %.1f s",
			spawner->spawnTimer, spawnInterval_);
		ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay);

		// 計算済み速度（読み取り専用）
		ImGui::Text("  連なり数: %d   計算速度: %.3f", spawner->chainCount, spawner->calculatedSpeed);
		ImGui::Separator();
	}

	if (idx == 0) {
		ImGui::TextDisabled("(スポナーが存在しません)");
	}

	ImGui::End();
}
#endif
