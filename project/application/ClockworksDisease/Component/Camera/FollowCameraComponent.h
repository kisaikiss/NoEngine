#pragma once
#include "engine/NoEngine.h"

struct FollowCameraComponent {
	float maxDistance = 15.0f;
	float minDistance = 2.0f;
	float distance = 15.0f;
	float theta = 0.f;
	float phi = 1.f;

	float moveSpeed = 2.0f;
	
	float minPhi = PI / 5.0f;
	float maxPhi = PI / 2.0f;

	float minFov = 0.65f;
	float maxFov = 0.85f;
	float playerPosOffset = 2.0f;

	// これより下はEditorに表示しない
	float currentDistance = maxDistance; // 実際にカメラが使う距離(地形で短縮される)
	float collisionRadius = 0.95f;       // カメラの太さ(壁の角へのめり込み防止)
	float collisionMargin = 0.3f;        // 壁面からの余白
	float collisionStartOffset = 5.0f; // プレイヤーから何m分カメラ側にずらして判定を始めるか
};