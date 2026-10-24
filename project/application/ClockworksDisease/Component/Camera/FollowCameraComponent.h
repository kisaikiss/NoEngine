#pragma once
#include "engine/NoEngine.h"

struct FollowCameraComponent {
	float maxDistance = 15.0f;
	float minDistance = 2.0f;
	float distance = 15.0f;
	float theta = 0.f;
	float phi = 1.f;

	float rotationSpeed = 2.0f;
	
	float minPhi = PI / 5.0f;
	float maxPhi = PI / 2.0f;

	float minFov = 0.65f;
	float maxFov = 0.85f;
	float playerPosOffset = 2.0f;

	float moveSpeed = 10.0f;
	float playerViewSpeed = 1.0f;
	float goalViewSpeed = 10.0f;

	// これより下はEditorに表示しない
	float currentDistance = maxDistance; // 実際にカメラが使う距離(地形で短縮される)
	float collisionRadius = 0.95f;       // カメラの太さ(壁の角へのめり込み防止)
	float collisionMargin = 0.3f;        // 壁面からの余白
	float collisionStartOffset = 5.0f; // プレイヤーから何m分カメラ側にずらして判定を始めるか
	float heightRiseFollowSpeed = 2.0f; // 高台へ上がる時の高さ追従速度。小さいほどゆっくり動く
	float heightFallFollowSpeed = 12.0f; // 下へ降りる時の高さ追従速度。大きいほど早く追いつく
	float heightRiseCatchUpThreshold = 4.0f; // ジャンプ中、この高さ以上上昇したらカメラも追従する
	float lookUpCameraClearance = 1.25f; // 見上げる角度の時にプレイヤーより上へ保つカメラの高さ
	float followHeight = 0.0f; // ジャンプ中の上下移動でカメラが追いかけないための追従基準高さ
	bool followHeightInitialized = false;
	bool followHeightAirborneCatchUp = false;
	bool followHeightCatchUpInProgress = false;
	No::Quaternion nextRotation = No::Quaternion::IDENTITY; // 見たい方向の回転。滑らかに角度を変えるための変数
};
