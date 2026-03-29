#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"

/// <summary>
/// 敵生成要求。
/// レールイベント側は要求を積むだけにし、実際の生成は専用Systemが担当する。
/// </summary>
struct SpawnEnemyRequestComponent {
	RailEnemySpawnEventParams params;
};
