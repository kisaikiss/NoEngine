#include "stdafx.h"
#include "EditorManager.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Data/RailDataIO.h"
#include "application/CommentBout/Data/EnemyDataIO.h"
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/Data/SpeechBubbleDataIO.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyShooterComponent.h"
#include "application/CommentBout/Component/Enemy/SpawnEnemyRequestComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyBulletComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/CollisionDebugConfigComponent.h"
#include "application/CommentBout/Component/Editor/DebugShortcutStateComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/OutGame/PauseStateComponent.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
void DestroyRuntimeEnemyEntities(No::Registry& registry);
void MarkPastEventsFiredByDistance(RailCameraComponent& rail, float distance);
}

// ---------------------------------------------------------------------------
// Initialize
// ---------------------------------------------------------------------------

void EditorManager::Initialize(No::Registry& registry, No::Entity railCameraEntity)
{
	railCameraEntity_ = railCameraEntity;

	// stageNameBuffer / activeStageName をレールカメラのステージ名で初期化
	if (railCameraEntity_ != No::nullEntity) {
		auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
		if (rail && !rail->stageName.empty()) {
			std::snprintf(stageNameBuffer_,  sizeof(stageNameBuffer_),  "%s", rail->stageName.c_str());
			std::snprintf(activeStageName_,  sizeof(activeStageName_),  "%s", rail->stageName.c_str());
		}
	}
	if (stageNameBuffer_[0] == '\0') {
		std::snprintf(stageNameBuffer_, sizeof(stageNameBuffer_), "Stage_01");
		std::snprintf(activeStageName_, sizeof(activeStageName_), "Stage_01");
	}

	stageWrapperExists_ = LoadStageWrapper(activeStageName_);

	enemyPresets_ = EnemyDataIO::Load();
	enemyPresetsLoaded_ = true;
}

// ---------------------------------------------------------------------------
// DrawImGui — メインエントリ (毎フレーム NotSystemUpdate から呼ぶ)
// ---------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif
void EditorManager::DrawImGui(No::Registry& registry)
{
#ifdef USE_IMGUI
	// スポーンデバッグの変更検知 → 再生成
	if (showSpawnDebug_) {
		const int sig = ComputeSpawnEventSignature(registry);
		if (sig != lastSpawnDebugSig_) {
			CreateSpawnDebugEntities(registry);
			lastSpawnDebugSig_ = sig;
		}
	}

	ImGui::Begin("Editor##EditorManager");

	// ---- ステージ変更（デフォルト閉じ） ----
	if (ImGui::CollapsingHeader("ステージ変更")) {
		ImGui::SetNextItemWidth(160.0f);
		ImGui::InputText("StageName##buf", stageNameBuffer_, sizeof(stageNameBuffer_));
		ImGui::SameLine();
		if (ImGui::Button("ステージ名確定")) {
			std::snprintf(activeStageName_, sizeof(activeStageName_), "%s", stageNameBuffer_);
			if (railCameraEntity_ != No::nullEntity) {
				auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
				if (rail) {
					rail->stageName = activeStageName_;
				}
			}

			const bool exists = LoadStageWrapper(activeStageName_);
			if (exists) {
				LoadAll(registry);
				stageWrapperExists_ = true;
			} else {
				SaveAll(registry);
				LoadAll(registry);
				stageWrapperExists_ = LoadStageWrapper(activeStageName_);
			}
		}
		if (stageWrapperExists_) {
			ImGui::TextDisabled("ステージデータが見つかりました");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "新規ステージ (保存で作成)");
		}
		ImGui::TextDisabled("確定するまでファイル操作には影響しません");
	}

	// ---- 一括 IO ----
	ImGui::Text("使用中: %s", activeStageName_);
	ImGui::SameLine();
	if (ImGui::Button("すべて読込")) {
		LoadAll(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("すべて保存")) {
		SaveAll(registry);
	}

	PauseStateComponent* pauseState = nullptr;
	for (auto entity : registry.View<CBPauseStateTag, PauseStateComponent>()) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}
	if (pauseState) {
		isPauseEnabled_ = pauseState->editorForcePause;
	}

	// ---- ゲームポーズチェックボックス ----
	if (ImGui::Checkbox("ゲームポーズ", &isPauseEnabled_)) {
		if (pauseState) {
			pauseState->editorForcePause = isPauseEnabled_;
		}
		for (auto entity : registry.View<NoEngine::ECS::PauseComponent>()) {
			if (auto* pause = registry.GetComponent<NoEngine::ECS::PauseComponent>(entity)) {
				pause->isPause = isPauseEnabled_;
			}
		}
	}

	// ---- デバッグ表示 CollapsingHeader ----
	DebugShortcutStateComponent* shortcut = nullptr;
	for (auto se : registry.View<DebugShortcutStateComponent>()) {
		shortcut = registry.GetComponent<DebugShortcutStateComponent>(se);
		if (shortcut) {
			break;
		}
	}
	CommentBoutCollision::CollisionDebugConfigComponent* colConfig = nullptr;
	for (auto ce : registry.View<CommentBoutCollision::CollisionDebugConfigComponent>()) {
		colConfig = registry.GetComponent<CommentBoutCollision::CollisionDebugConfigComponent>(ce);
		if (colConfig) {
			break;
		}
	}
	RailCameraComponent* railForDebug = nullptr;
	if (railCameraEntity_ != No::nullEntity) {
		railForDebug = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	}

	if (shortcut && shortcut->applyDebugDisplayAll) {
		const bool enable = shortcut->debugDisplayAll;
		showSpawnDebug_ = enable;
		if (showSpawnDebug_) {
			CreateSpawnDebugEntities(registry);
			lastSpawnDebugSig_ = ComputeSpawnEventSignature(registry);
		} else {
			DestroySpawnDebugEntities(registry);
			lastSpawnDebugSig_ = -1;
		}

		if (railForDebug) {
			railForDebug->drawRailDebug = enable;
			railForDebug->drawCameraDebug = enable;
			railForDebug->drawControlPointsDebug = enable;
			railForDebug->drawEventPointsDebug = enable;
		}
		if (colConfig) {
			colConfig->enableCollisionDebug = enable;
			colConfig->showEnemyCollider = enable;
			colConfig->showEnemyBulletCollider = enable;
			colConfig->showFieldCollider = enable;
			colConfig->showPlayerHitboxOverlay = enable;
			colConfig->showCameraGateWire = enable;
			colConfig->showPlayerAttackCollider = enable;
		}
		shortcut->applyDebugDisplayAll = false;
	}
	if (ImGui::CollapsingHeader("デバッグ表示")) {
		// 1. スポーンデバッグ
		if (ImGui::Checkbox("スポーンデバッグ", &showSpawnDebug_)) {
			if (showSpawnDebug_) {
				CreateSpawnDebugEntities(registry);
				lastSpawnDebugSig_ = ComputeSpawnEventSignature(registry);
			} else {
				DestroySpawnDebugEntities(registry);
				lastSpawnDebugSig_ = -1;
			}
		}
		ImGui::SameLine();
		// 2. レールカメラ描画チェックボックス
		if (railForDebug) {
			ImGui::Checkbox("レール描画", &railForDebug->drawRailDebug);
			ImGui::SameLine();
			ImGui::Checkbox("カメラギズモ", &railForDebug->drawCameraDebug);
			ImGui::SameLine();
			ImGui::Checkbox("制御点", &railForDebug->drawControlPointsDebug);
			ImGui::SameLine();
			ImGui::Checkbox("イベント点", &railForDebug->drawEventPointsDebug);

			// 制御点半径とイベント点半径を同一値で管理
			if (ImGui::DragFloat("デバッグ半径", &railForDebug->controlPointDebugRadius, 0.005f, 0.01f, 1.0f)) {
				railForDebug->eventPointDebugRadius = railForDebug->controlPointDebugRadius;
			}
		}
		// 3. コリジョンデバッグ設定（CollisionDebugRenderSystem から移植）
		if (colConfig) {
			ImGui::Separator();
			ImGui::Checkbox("衝突デバッグ表示", &colConfig->enableCollisionDebug);
			if (colConfig->enableCollisionDebug) {
				ImGui::Indent();
				ImGui::Checkbox("敵コライダー",        &colConfig->showEnemyCollider);
				ImGui::Checkbox("敵弾コライダー",      &colConfig->showEnemyBulletCollider);
				ImGui::Checkbox("フィールドコライダー",&colConfig->showFieldCollider);
				ImGui::Checkbox("自機ヒットボックス",  &colConfig->showPlayerHitboxOverlay);
				ImGui::Checkbox("カメラゲート",        &colConfig->showCameraGateWire);
				ImGui::Checkbox("攻撃コライダー",      &colConfig->showPlayerAttackCollider);
				ImGui::Unindent();
			}
		}
	}

	ImGui::Separator();

	// ---- タブバー ----
	if (ImGui::BeginTabBar("EditorTabs")) {
		if (ImGui::BeginTabItem("レール")) {
			DrawRailTab(registry);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("イベント")) {
			DrawEventTab(registry);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("フィールド")) {
			DrawFieldTab(registry);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("敵設定")) {
			DrawEnemyTab(registry);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("フィールドオブジェクト設定")) {
			DrawFieldTypeDefaultsTab(registry);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("吹き出し")) {
			DrawSpeechBubbleTab(registry);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// 一括 IO
// ---------------------------------------------------------------------------

void EditorManager::LoadAll(No::Registry& registry)
{
	if (railCameraEntity_ == No::nullEntity) return;
	auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) return;

	const std::string stageName(activeStageName_);
	if (stageName.empty()) return;

	rail->stageName = stageName;
	rail->railFilePath = MakeRailFilePath(stageName);
	LoadRailToComponent(*rail, stageName);
	LoadEventsToComponent(*rail, stageName);
	DestroyRuntimeEnemyEntities(registry);

	// FieldObjectEditor は次の Update() で自動リロード
	fieldObjectEditor_.ForceReload();

	// スポーンデバッグを再生成
	if (showSpawnDebug_) {
		CreateSpawnDebugEntities(registry);
		lastSpawnDebugSig_ = ComputeSpawnEventSignature(registry);
	}
}

void EditorManager::SaveAll(No::Registry& registry)
{
	if (railCameraEntity_ == No::nullEntity) return;
	auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) return;

	const std::string stageName(activeStageName_);
	if (stageName.empty()) return;

	rail->stageName = stageName;
	SaveStageWrapper(stageName);
	SaveRailToJson(*rail, stageName);
	SaveEventsToJson(*rail, stageName);
	fieldObjectEditor_.Save(registry);
}

// ---------------------------------------------------------------------------
// スポーンデバッグエンティティ
// ---------------------------------------------------------------------------

int EditorManager::ComputeSpawnEventSignature(No::Registry& registry) const
{
	if (railCameraEntity_ == No::nullEntity) return 0;
	const auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) return 0;

	int sig = static_cast<int>(rail->events.size()) * 97;
	for (const auto& e : rail->events) {
		if (e.type != RailEventType::SpawnEnemy) continue;
		sig += static_cast<int>(e.spawn.count) * 31;
		sig += static_cast<int>(e.spawn.spawnPosition.x * 7.0f);
		sig += static_cast<int>(e.spawn.spawnPosition.y * 11.0f);
		sig += static_cast<int>(e.spawn.spawnPosition.z * 13.0f);
		sig += static_cast<int>(e.spawn.moveDirection.x * 17.0f);
		sig += static_cast<int>(e.spawn.moveDirection.y * 19.0f);
		sig += static_cast<int>(e.spawn.moveDirection.z * 23.0f);
		sig += static_cast<int>(e.triggerDistance * 5.0f);
		sig += static_cast<int>(e.spawn.enemyType) * 37;
		sig += static_cast<int>(e.spawn.spawnOffset.x * 41.0f);
		sig += static_cast<int>(e.spawn.spawnOffset.y * 43.0f);
		sig += static_cast<int>(e.spawn.spawnOffset.z * 47.0f);
	}
	return sig;
}

void EditorManager::CreateSpawnDebugEntities(No::Registry& registry)
{
	DestroySpawnDebugEntities(registry);

	if (railCameraEntity_ == No::nullEntity) return;
	auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) return;

	GameResourceComponent* resources = nullptr;
	for (auto entity : registry.View<CBGameResourceTag, GameResourceComponent>()) {
		resources = registry.GetComponent<GameResourceComponent>(entity);
		if (resources) break;
	}
	if (!resources) return;

	for (const auto& event : rail->events) {
		if (event.type != RailEventType::SpawnEnemy) continue;
		const auto& sp = event.spawn;

		No::Vector3 dir = sp.moveDirection;
		const float dirLen = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
		const bool hasDir = dirLen > 0.0001f;
		if (hasDir) { dir.x /= dirLen; dir.y /= dirLen; dir.z /= dirLen; }

		for (int i = 0; i < sp.count; ++i) {
			const No::Vector3 pos = sp.spawnPosition + sp.spawnOffset * static_cast<float>(i);

			No::Entity e = registry.GenerateEntity();
			auto* transform = registry.AddComponent<No::TransformComponent>(e);
			transform->translate = pos;
			if (hasDir) {
				No::Quaternion rot;
				rot.LookRotation(dir, No::Vector3::UP);
				transform->rotation = rot;
			}

			const std::string& modelKey = (sp.enemyType == RailEnemyType::Boss)
				? CommentBoutResourceKey::kBossModel
				: CommentBoutResourceKey::kEnemyModel;

			auto* mesh = registry.AddComponent<No::MeshComponent>(e);
			auto* material = registry.AddComponent<No::MaterialComponent>(e);
			if (const auto* model = FindGameModel(*resources, modelKey)) {
				NoEngine::Asset::ModelLoader::GetModel(model->assetName, mesh);
				material->materials = NoEngine::Asset::ModelLoader::GetMaterial(model->assetName);
			}
			material->psoName = L"Renderer : Default PSO";
			material->psoId = NoEngine::Render::GetPSOID(material->psoName);
			material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
			material->color = No::Color(0.f, 0.f, 0.f, 1.f);

			spawnDebugEntities_.push_back(e);
		}
	}
}

void EditorManager::DestroySpawnDebugEntities(No::Registry& registry)
{
	for (auto e : spawnDebugEntities_) {
		registry.DestroyEntity(e);
	}
	spawnDebugEntities_.clear();
}

// ---------------------------------------------------------------------------
// タブ描画
// ---------------------------------------------------------------------------

void EditorManager::DrawRailTab(No::Registry& registry)
{
#ifdef USE_IMGUI
	if (railCameraEntity_ == No::nullEntity) {
		ImGui::TextDisabled("レールカメラエンティティが未設定です");
		return;
	}
	auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) {
		ImGui::TextDisabled("RailCameraComponent が見つかりません");
		return;
	}

	const std::string stageName(activeStageName_);
	if (ImGui::Button("JSONから再読み込み") && !stageName.empty()) {
		DestroyRuntimeEnemyEntities(registry);
		rail->stageName = stageName;
		rail->railFilePath = MakeRailFilePath(stageName);
		LoadRailToComponent(*rail, stageName);
	}
	ImGui::SameLine();
	if (ImGui::Button("JSONへ保存") && !stageName.empty()) {
		rail->stageName = stageName;
		rail->railFilePath = MakeRailFilePath(stageName);
		SaveRailToJson(*rail, stageName);
	}

	// 制御点編集	
	ImGui::SeparatorText("制御点編集");
	railCameraEditor_.DrawRailEditorContent(&registry, railCameraEntity_);
	ImGui::Separator();
	// レールカメラ情報・再生コントロール
	if (ImGui::CollapsingHeader("制御点編集", ImGuiTreeNodeFlags_DefaultOpen)) {
		railCameraEditor_.DrawRailCameraContent(&registry, railCameraEntity_, &ResetEventRuntime);
	}



#else
	static_cast<void>(registry);
#endif
}

void EditorManager::DrawEventTab(No::Registry& registry)
{
#ifdef USE_IMGUI
	const std::string stageName(activeStageName_);
	auto* rail = registry.GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) {
		ImGui::TextDisabled("RailCameraComponent が見つかりません");
		return;
	}
	if (ImGui::Button("JSONから再読み込み") && !stageName.empty()) {
		const float currentDistance = rail->distance;
		LoadEventsToComponent(*rail, stageName);
		MarkPastEventsFiredByDistance(*rail, currentDistance);
	}
	ImGui::SameLine();
	if (ImGui::Button("JSONへ保存") && !stageName.empty()) {
		rail->stageName = stageName;
		SaveEventsToJson(*rail, stageName);
	}
	ImGui::Separator();
	gameEventEditor_.DrawGameEventEditorImGui(&registry, railCameraEntity_);
#else
	static_cast<void>(registry);
#endif
}

void EditorManager::DrawFieldTab(No::Registry& registry)
{
	fieldObjectEditor_.DrawImGui(registry);
}

void EditorManager::DrawEnemyTab(No::Registry& registry)
{
#ifdef USE_IMGUI
	if (!enemyPresetsLoaded_) {
		enemyPresets_ = EnemyDataIO::Load();
		enemyPresetsLoaded_ = true;
	}

	if (ImGui::Button("JSONから再読み込み")) {
		enemyPresets_ = EnemyDataIO::Load();
		ApplyEnemyPresetsToAliveEnemies(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("JSONへ保存")) {
		EnemyDataIO::Save(enemyPresets_);
	}

	ImGui::Separator();

	auto drawType = [&](RailEnemyType type, const char* label) {
		EnemyConfig cfg = EnemyDataIO::GetOrDefault(enemyPresets_, type);
		if (!ImGui::TreeNode(label)) return;
		bool changed = false;

		ImGui::SeparatorText("外観");
		changed |= ImGui::DragFloat("モデルスケール", &cfg.modelScale, 0.01f, 0.1f, 10.0f);
		ImGui::SeparatorText("弾モデル");
		changed |= ImGui::DragFloat("弾モデルスケール", &cfg.bulletModelScale, 0.001f, 0.01f, 2.0f);
		ImGui::SeparatorText("当たり判定");
		changed |= ImGui::DragFloat3("コライダーサイズ(Box)", &cfg.enemyCollider.boxSizeMultiplier.x, 0.01f, 0.01f, 20.0f);
		changed |= ImGui::DragFloat3("コライダーオフセット", &cfg.enemyCollider.localOffset3D.x, 0.01f, -10.0f, 10.0f);
		ImGui::SeparatorText("ステータス");
		changed |= ImGui::DragInt("HP", &cfg.minHp, 1, 1, 9999);
		changed |= ImGui::DragFloat("デスポーン距離", &cfg.despawnBehindDistance, 0.5f, 0.0f, 200.0f);
		ImGui::SeparatorText("射撃");
		changed |= ImGui::DragFloat("射撃間隔(秒)", &cfg.shootInterval, 0.01f, 0.05f, 10.0f);
		changed |= ImGui::DragFloat("弾速", &cfg.bulletSpeed, 0.05f, 0.1f, 100.0f);
		changed |= ImGui::DragInt("弾ダメージ", &cfg.bulletDamage, 1, 1, 999);
		changed |= ImGui::DragFloat("弾の寿命(秒)", &cfg.bulletLifetime, 0.05f, 0.1f, 30.0f);
		changed |= ImGui::DragFloat("カメラからの射撃深度", &cfg.targetDepthFromCamera, 0.01f, 0.1f, 20.0f);
		changed |= ImGui::DragFloat("射撃最大距離", &cfg.shootDistanceMax, 0.05f, 0.0f, 200.0f);
		ImGui::SeparatorText("弾コライダー");
		changed |= ImGui::DragFloat("弾コライダー半径倍率", &cfg.bulletCollider.radiusMultiplier, 0.01f, 0.01f, 5.0f);
		changed |= ImGui::DragFloat3("弾コライダーオフセット", &cfg.bulletCollider.localOffset3D.x, 0.01f, -5.0f, 5.0f);

		enemyPresets_[type] = cfg;
		if (changed) ApplyEnemyPresetsToAliveEnemies(registry);
		ImGui::TreePop();
		};

	drawType(RailEnemyType::MoveOnly, "通常敵 (MoveOnly)");
	drawType(RailEnemyType::MoveAndShoot, "射撃敵 (MoveAndShoot)");
	drawType(RailEnemyType::Boss, "ボス (Boss)");
#else
	static_cast<void>(registry);
#endif
}

void EditorManager::DrawFieldTypeDefaultsTab(No::Registry& registry)
{
	fieldObjectEditor_.DrawTypeDefaultsImGui(registry);
}

void EditorManager::DrawSpeechBubbleTab(No::Registry& registry)
{
#ifdef USE_IMGUI
	SpeechBubbleConfig* cfg = nullptr;
	for (auto e : registry.View<CBSpeechBubbleConfigTag, SpeechBubbleConfig>()) {
		cfg = registry.GetComponent<SpeechBubbleConfig>(e);
		if (cfg) break;
	}
	if (!cfg) { ImGui::TextDisabled("SpeechBubbleConfig エンティティが見つかりません"); return; }

	if (ImGui::Button("JSONから再読み込み")) {
		*cfg = SpeechBubbleDataIO::Load();
	}
	ImGui::SameLine();
	if (ImGui::Button("JSONへ保存")) {
		SpeechBubbleDataIO::Save(*cfg);
	}
	ImGui::Separator();

	ImGui::SeparatorText("距離閾値（カメラからの3D距離）");
	ImGui::DragFloat("大サイズ 最大距離", &cfg->largeMaxDistance, 0.1f, 0.1f, 100.f);
	ImGui::DragFloat("中サイズ 最大距離", &cfg->mediumMaxDistance, 0.1f, 0.1f, 100.f);
	if (cfg->largeMaxDistance >= cfg->mediumMaxDistance) {
		cfg->largeMaxDistance = cfg->mediumMaxDistance - 0.1f;
	}
	ImGui::TextDisabled("中距離 〜 ∞ → 小サイズ");

	ImGui::SeparatorText("出現演出（共通）");
	ImGui::DragFloat("出現開始スケール比率", &cfg->initialScalePercent, 0.01f, 0.05f, 1.0f);
	ImGui::TextDisabled("0.3 = 最大サイズの 30% から出現");

	ImGui::Separator();
	auto editSize = [](const char* label, SpeechBubbleSizeConfig& s) {
		if (!ImGui::TreeNode(label)) return;
		ImGui::DragFloat2("スプライトサイズ", &s.spriteSize.x, 1.f, 4.f, 512.f);
		ImGui::DragInt("ダメージ量", &s.attackPower, 1, 1, 100);
		ImGui::DragFloat("飛翔時間(秒)", &s.duration, 0.01f, 0.1f, 5.f);
		ImGui::SeparatorText("出現演出");
		ImGui::DragFloat("出現イージング時間(秒)", &s.appearDuration, 0.01f, 0.05f, 3.0f);
		ImGui::DragFloat("停止時間(秒)",           &s.stopDuration,   0.01f, 0.0f,  5.0f);
		ImGui::TreePop();
		};
	editSize("大 (POW.png)", cfg->sizeLarge);
	editSize("中 (BOOM.png)", cfg->sizeMedium);
	editSize("小 (OH.png)", cfg->sizeSmall);
#else
	static_cast<void>(registry);
#endif
}

// ---------------------------------------------------------------------------
// ApplyEnemyPresetsToAliveEnemies
// ---------------------------------------------------------------------------

void EditorManager::ApplyEnemyPresetsToAliveEnemies(No::Registry& registry)
{
	for (auto entity : registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, No::TransformComponent>()) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!enemy || !health || !transform) continue;

		RailEnemyType enemyType = RailEnemyType::MoveOnly;
		if (registry.Has<CBBossTag>(entity))              enemyType = RailEnemyType::Boss;
		else if (registry.Has<EnemyShooterComponent>(entity)) enemyType = RailEnemyType::MoveAndShoot;

		const EnemyConfig cfg = EnemyDataIO::GetOrDefault(enemyPresets_, enemyType);

		transform->scale = { cfg.modelScale, cfg.modelScale, cfg.modelScale };
		enemy->despawnBehindDistance = std::max(0.0f, cfg.despawnBehindDistance);
		enemy->maxHp = std::max(1, cfg.minHp);
		enemy->hp = std::min(enemy->hp, enemy->maxHp);
		health->maxHp = enemy->maxHp;
		health->hp = std::min(health->hp, health->maxHp);

		if (auto* rs = registry.GetComponent<EnemyRewardSourceComponent>(entity)) rs->worldSizeForReward = cfg.modelScale;
		if (auto* col = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity)) {
			col->shapeType = CommentBoutCollision::ShapeType3D::Box;
			col->useScaleAsBox = true;
			col->boxSizeMultiplier = {
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.x),
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.y),
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.z)
			};
			col->localOffset = cfg.enemyCollider.localOffset3D;
		}
		if (auto* shooter = registry.GetComponent<EnemyShooterComponent>(entity)) {
			shooter->shootInterval = std::max(0.05f, cfg.shootInterval);
			shooter->bulletSpeed = std::max(0.1f, cfg.bulletSpeed);
			shooter->bulletDamage = std::max(1, cfg.bulletDamage);
			shooter->targetDepthFromCamera = std::max(0.1f, cfg.targetDepthFromCamera);
			shooter->bulletLifetime = std::max(0.1f, cfg.bulletLifetime);
			shooter->shootDistanceMax = std::max(0.0f, cfg.shootDistanceMax);
			shooter->bulletModelScale               = std::max(0.01f, cfg.bulletModelScale);
			shooter->bulletColliderRadiusMultiplier = std::max(0.01f, cfg.bulletCollider.radiusMultiplier);
			shooter->bulletColliderLocalOffset      = cfg.bulletCollider.localOffset3D;
		}
	}
}

namespace {
void DestroyRuntimeEnemyEntities(No::Registry& registry) {
	std::vector<No::Entity> entities;
	for (auto e : registry.View<CBRailEnemyTag>()) {
		entities.push_back(e);
	}
	for (auto e : registry.View<CBEnemyBulletTag>()) {
		entities.push_back(e);
	}
	for (auto e : registry.View<SpawnEnemyRequestComponent>()) {
		entities.push_back(e);
	}
	for (auto e : entities) {
		registry.DestroyEntity(e);
	}
}

void MarkPastEventsFiredByDistance(RailCameraComponent& rail, float distance) {
	ResetEventRuntime(rail);
	if (distance < 0.0f) {
		distance = 0.0f;
	}

	for (size_t i = 0; i < rail.events.size(); ++i) {
		auto& eventData = rail.events[i];
		if (eventData.triggerDistance > distance) {
			continue;
		}

		switch (eventData.type) {
		case RailEventType::SpawnEnemy:
			eventData.fired = true;
			break;
		case RailEventType::RailStop:
			eventData.fired = true;
			rail.runtimeState = RailRuntimeState::Stopped;
			rail.isPlaying = false;
			rail.waitingSourceEventIndex = static_cast<int>(i);
			if (eventData.resumeCondition != RailResumeConditionType::None) {
				rail.runtimeState = RailRuntimeState::WaitingResumeCondition;
				rail.waitingResumeCondition = eventData.resumeCondition;
				rail.waitingResumeAfterSeconds = std::max(0.0f, eventData.resumeAfterSeconds);
				rail.waitingTargetGroupId = eventData.targetGroupId;
				rail.waitingResumeElapsedSeconds = 0.0f;
			}
			break;
		}
	}

	rail.distance = distance;
}
}
