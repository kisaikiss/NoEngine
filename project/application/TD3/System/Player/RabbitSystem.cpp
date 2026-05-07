#include "application/TD3/System/Player/RabbitSystem.h"
#include "application/TD3/Component/Player/RabbitComponent.h"
#include "application/TD3/Component/GravityComponent.h"
#include "application/TD3/Stage/MapData.h"
#include "application/TD3/Stage/MapManager.h"
#include "application/TD3/Stage/GameStateComponent.h"
#include <algorithm>
#include <cmath>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
#ifdef USE_IMGUI
	//自機のimgui表示
	void DrawPlayerDebugWindow(No::Registry& registry, No::Entity playerEntity) {
		auto* p = registry.GetComponent<RabbitComponent>(playerEntity);
		if (!p) return;

		ImGui::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(230.f, 90.f), ImGuiCond_FirstUseEver);
		ImGui::Begin("Player Debug");

		ImGui::Text("Grounded    : %s", p->isGrounded ? "true" : "false");
		ImGui::Text("WallSliding : %s", p->isWallSliding ? "true" : "false");
		ImGui::Text("WallL/R     : %s / %s",
			p->isTouchingWallLeft ? "true" : "false",
			p->isTouchingWallRight ? "true" : "false");
		ImGui::Separator();
		ImGui::Text("Respawn: (%.0f, %.0f)", p->respawnX, p->respawnY);
		ImGui::Text("RespawnMap: %s", p->respawnMap.empty() ? "(none)" : p->respawnMap.c_str());
		if (ImGui::Button("Kill Player")) {
			p->isDead = true;
		}

		ImGui::End();
	}
#endif
}

void RabbitSystem::Initialize(No::Entity managerEntity, No::Entity playerEntity,
	No::Entity cam2dEntity, Stage::MapManager* mapManager) {
	managerEntity_ = managerEntity;
	playerEntity_ = playerEntity;
	cam2dEntity_ = cam2dEntity;
	mapMgr_ = mapManager;
}

void RabbitSystem::Update(No::Registry& registry, float dt) {
#ifdef USE_IMGUI
	DrawPlayerDebugWindow(registry, playerEntity_);
#endif

	auto* state = registry.GetComponent<GameStateComponent>(managerEntity_);
	if (!state || state->editorMode) return;

	// マップが切り替わっていたら respawnX/Y を initialSpawn で初期化
	SyncRespawnToInitialSpawn(registry);

	auto* p = registry.GetComponent<RabbitComponent>(playerEntity_);
	auto* t = registry.GetComponent<No::Transform2DComponent>(playerEntity_);

	// 死亡処理: isDead == true のフレームでリスポーン地点へワープ
	if (p && t && p->isDead) {
		auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
		if (mapMgr_ && !p->respawnMap.empty() && (!mapData || mapData->GetMapName() != p->respawnMap)) {
			if (mapMgr_->LoadMap(registry, managerEntity_, p->respawnMap)) {
				auto* loadedMapData = registry.GetComponent<Stage::MapData>(managerEntity_);
				if (loadedMapData) {
					lastStageNo_ = loadedMapData->stageNo;
				}
			}
		}

		t->translate = { p->respawnX, p->respawnY };
		p->isDead = false;
		lockedTransitionGx_ = -1;
		lockedTransitionGy_ = -1;
		UpdateCamera(registry);
		return;
	}

	Move(registry, dt);
	UpdateCamera(registry);
	CheckRespawnPoint(registry);
	CheckTransition(registry);
}

void RabbitSystem::Move(No::Registry& registry, float dt) {
	auto* t = registry.GetComponent<No::Transform2DComponent>(playerEntity_);
	auto* p = registry.GetComponent<RabbitComponent>(playerEntity_);
	auto* grav = registry.GetComponent<GravityComponent>(playerEntity_);
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!t || !p || !mapData) return;

	const float hw = p->width * 0.5f;
	const float hh = p->height * 0.5f;
	const float TS = static_cast<float>(Stage::TILE_SIZE);

	auto isSolid = [&](int gx, int gy) -> bool {
		if (!mapData->InBounds(gx, gy)) return true;
		return !mapData->fieldTiles[gy][gx].IsEmpty();
		};

	// ----------------------------------------------------------------
	// X 軸: 加速度・摩擦による慣性移動 → タイル衝突解決
	// ----------------------------------------------------------------
	if (grav) {
		bool pressingD = No::Keyboard::IsPress('D');
		bool pressingA = No::Keyboard::IsPress('A');

		if (pressingD || pressingA) {
			// 入力あり: 目標速度へ向けて加速
			float targetVX = 0.f;
			if (pressingD) targetVX += p->speed;
			if (pressingA) targetVX -= p->speed;

			float delta = targetVX - grav->velocityX;
			float maxDelta = p->accelX * dt;
			if (std::abs(delta) <= maxDelta) {
				grav->velocityX = targetVX;
			} else {
				grav->velocityX += maxDelta * (delta > 0.f ? 1.f : -1.f);
			}
		} else {
			// 入力なし: 摩擦で 0 へ減速
			float friction = p->frictionX * dt;
			if (std::abs(grav->velocityX) <= friction) {
				grav->velocityX = 0.f;
			} else {
				grav->velocityX -= friction * (grav->velocityX > 0.f ? 1.f : -1.f);
			}
		}

		t->translate.x += grav->velocityX * dt;
	}

	// X 衝突解決
	if (grav && grav->velocityX != 0.f) {
		float top = t->translate.y - hh;
		float bottom = t->translate.y + hh;
		int gyMin = static_cast<int>(top / TS);
		int gyMax = static_cast<int>((bottom - 0.01f) / TS);

		if (grav->velocityX > 0.f) {
			float right = t->translate.x + hw;
			int gx = static_cast<int>((right - 0.01f) / TS);
			for (int gy = gyMin; gy <= gyMax; ++gy) {
				if (isSolid(gx, gy)) {
					t->translate.x = gx * TS - hw;
					grav->velocityX = 0.f;
					break;
				}
			}
		} else {
			float left = t->translate.x - hw;
			int gx = static_cast<int>(left / TS);
			for (int gy = gyMin; gy <= gyMax; ++gy) {
				if (isSolid(gx, gy)) {
					t->translate.x = (gx + 1) * TS + hw;
					grav->velocityX = 0.f;
					break;
				}
			}
		}
	}

	// ----------------------------------------------------------------
	// 壁への接触を検出（1px 外側のタイルを探索）
	// ----------------------------------------------------------------
	p->isTouchingWallLeft = false;
	p->isTouchingWallRight = false;
	{
		float top = t->translate.y - hh;
		float bottom = t->translate.y + hh;
		int gyMin = static_cast<int>(top / TS);
		int gyMax = static_cast<int>((bottom - 0.01f) / TS);
		int gxLeft = static_cast<int>((t->translate.x - hw - 1.f) / TS);
		int gxRight = static_cast<int>((t->translate.x + hw) / TS);
		for (int gy = gyMin; gy <= gyMax; ++gy) {
			if (isSolid(gxLeft, gy)) p->isTouchingWallLeft = true;
			if (isSolid(gxRight, gy)) p->isTouchingWallRight = true;
		}
	}

	// ----------------------------------------------------------------
	// 壁ずり判定（空中・落下中・壁方向に入力中）
	// ----------------------------------------------------------------
	p->isWallSliding = false;
	if (grav && !p->isGrounded && grav->velocityY > 0.f) {
		bool pressingLeft = No::Keyboard::IsPress('A');
		bool pressingRight = No::Keyboard::IsPress('D');
		if ((p->isTouchingWallLeft && pressingLeft) ||
			(p->isTouchingWallRight && pressingRight)) {
			p->isWallSliding = true;
		}
	}

	// ----------------------------------------------------------------
	// Y 軸移動（重力 + ジャンプ + 壁ジャンプ） → タイル衝突解決
	// ----------------------------------------------------------------
	if (grav) {
		// 通常ジャンプ（接地時）
		if (p->isGrounded && No::Keyboard::IsTrigger('W')) {
			grav->velocityY = -p->jumpSpeed;
		}
		// 壁ジャンプ（壁ずり中）
		// velocityX に大きな横初速を与える。慣性があるため自然に減速する。
		else if (p->isWallSliding && No::Keyboard::IsTrigger('W')) {
			grav->velocityY = -p->jumpSpeed;
			grav->velocityX = p->isTouchingWallLeft
				? p->wallJumpSpeedX  // 左壁 → 右へ
				: -p->wallJumpSpeedX; // 右壁 → 左へ
		}

		// 重力加速（壁ずり中は落下速度を抑える）
		float effectiveMaxFall = p->isWallSliding ? p->wallSlideSpeed : grav->maxFallSpeed;
		grav->velocityY += grav->gravity * dt;
		grav->velocityY = std::min(grav->velocityY, effectiveMaxFall);

		t->translate.y += grav->velocityY * dt;
	}

	// Y 衝突解決
	p->isGrounded = false;
	if (grav) {
		float left = t->translate.x - hw;
		float right = t->translate.x + hw;
		int gxMin = static_cast<int>(left / TS);
		int gxMax = static_cast<int>((right - 0.01f) / TS);

		if (grav->velocityY >= 0.f) {
			float bottom = t->translate.y + hh;
			int gy = static_cast<int>((bottom - 0.01f) / TS);
			for (int gx = gxMin; gx <= gxMax; ++gx) {
				if (isSolid(gx, gy)) {
					t->translate.y = gy * TS - hh;
					grav->velocityY = 0.f;
					p->isGrounded = true;
					break;
				}
			}
		} else {
			float top = t->translate.y - hh;
			int gy = static_cast<int>(top / TS);
			for (int gx = gxMin; gx <= gxMax; ++gx) {
				if (isSolid(gx, gy)) {
					t->translate.y = (gy + 1) * TS + hh;
					grav->velocityY = 0.f;
					break;
				}
			}
		}
	}
}

void RabbitSystem::UpdateCamera(No::Registry& registry) {
	auto* playerT = registry.GetComponent<No::Transform2DComponent>(playerEntity_);
	auto* camT    = registry.GetComponent<No::Transform2DComponent>(cam2dEntity_);
	if (!playerT || !camT) return;

	float camX = playerT->translate.x - 1280.f * 0.5f;
	float camY = playerT->translate.y - 720.f * 0.5f;

	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (mapData && mapMgr_) {
		auto maxCam = mapMgr_->GetMaxCameraOffset(*mapData);
		camX = std::clamp(camX, 0.f, maxCam.x);
		camY = std::clamp(camY, 0.f, maxCam.y);
	}

	camT->translate.x = camX;
	camT->translate.y = camY;
}

void RabbitSystem::SyncRespawnToInitialSpawn(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;
	auto* p = registry.GetComponent<RabbitComponent>(playerEntity_);
	if (!p) return;

	if (mapData->stageNo == lastStageNo_) {
		if (p->respawnMap.empty()) {
			p->respawnMap = mapData->GetMapName();
		}
		return;
	}

	lastStageNo_ = mapData->stageNo;
	p->respawnX = static_cast<float>(mapData->initialSpawn.x * Stage::TILE_SIZE + Stage::TILE_SIZE / 2);
	p->respawnY = static_cast<float>(mapData->initialSpawn.y * Stage::TILE_SIZE + Stage::TILE_SIZE / 2);
	p->respawnMap = mapData->GetMapName();
}

void RabbitSystem::CheckRespawnPoint(No::Registry& registry) {
	auto* t = registry.GetComponent<No::Transform2DComponent>(playerEntity_);
	auto* p = registry.GetComponent<RabbitComponent>(playerEntity_);
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!t || !p || !mapData) return;

	int gx = static_cast<int>(t->translate.x / Stage::TILE_SIZE);
	int gy = static_cast<int>(t->translate.y / Stage::TILE_SIZE);

	for (const auto& rp : mapData->respawnPoints) {
		if (rp.x == gx && rp.y == gy) {
			float newX = static_cast<float>(rp.x * Stage::TILE_SIZE + Stage::TILE_SIZE / 2);
			float newY = static_cast<float>(rp.y * Stage::TILE_SIZE + Stage::TILE_SIZE / 2);
			p->respawnX = newX;
			p->respawnY = newY;
			p->respawnMap = mapData->GetMapName();
			break;
		}
	}
}

void RabbitSystem::CheckTransition(No::Registry& registry) {
	if (!mapMgr_) return;
	auto* t = registry.GetComponent<No::Transform2DComponent>(playerEntity_);
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!t || !mapData) return;

	int currentGx = static_cast<int>(t->translate.x / Stage::TILE_SIZE);
	int currentGy = static_cast<int>(t->translate.y / Stage::TILE_SIZE);
	if (lockedTransitionGx_ >= 0 && lockedTransitionGy_ >= 0) {
		if (currentGx == lockedTransitionGx_ && currentGy == lockedTransitionGy_) {
			return;
		}
		lockedTransitionGx_ = -1;
		lockedTransitionGy_ = -1;
	}

	const auto* tp = mapMgr_->CheckTransition(*mapData, t->translate.x, t->translate.y);
	if (!tp) return;

	No::Vector2 spawnPos;
	if (mapMgr_->TransitionTo(registry, managerEntity_, tp->targetMap, tp->targetPoint, spawnPos)) {
		t->translate = { spawnPos.x, spawnPos.y };
		lockedTransitionGx_ = static_cast<int>(spawnPos.x / Stage::TILE_SIZE);
		lockedTransitionGy_ = static_cast<int>(spawnPos.y / Stage::TILE_SIZE);
	}
}
