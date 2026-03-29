#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"

enum class BossPhase {
	Move = 0,
	Stop = 1,
	Shoot = 2
};

/// <summary>
/// ボス挙動パラメータとランタイム状態を保持するコンポーネント。
/// </summary>
struct BossComponent {
	BossBehaviorParams behavior{};

	BossPhase phase = BossPhase::Move;
	float phaseTime = 0.0f;
	float shotTimer = 0.0f;
	int shotsFiredInBurst = 0;
	int shotsSpawnedInBurst = 0;
	No::Vector3 worldOrigin = No::Vector3::ZERO;
};
