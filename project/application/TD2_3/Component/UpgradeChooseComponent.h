#pragma once
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include <array>
#include <vector>
#include <string>

struct UpgradeChooseComponent
{
	// 管理するスプライトエンティティ（3つが選択肢）
	std::array<No::Entity, 3> optionEntities{};
	// 各選択肢用の buttonFrame を 3 つ持つ
	std::array<No::Entity, 3> frameEntities{};

	// optionEntities に対応する sprite 名（hpLimitUp / ballUp / paddleSpread）
	std::array<std::string, 3> optionNames{};

	// 表示中フラグ（status->pendingUpgrade を監視して制御）
	bool active = false;

	// 選択インデックス（0..2）
	int selectedIndex = 1;

	// 各オプションのアニメーションオフセット（Y方向）
	std::array<float, 3> animOffset{ 0.f, 0.f, 0.f };

	// ベース Y（画面中央 Y を保持）
	float baseY = 0.f;

	// 各オプションのベース X（画面中央からの配置）
	std::array<float, 3> baseX{};

	// 入力デバウンス
	float inputCooldown = 0.f;

	// マウス状態保存
	bool wasMouseDown = false;
};