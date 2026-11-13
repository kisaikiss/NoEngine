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
	bool autoFollowEnabled = true;
	float autoFollowDelay = 1.0f; // 手動操作後、自動回転を再開するまでの秒数
	float autoFollowDeadzoneAngle = 0.15f; // 小さな進行方向の変化には反応しない
	float maxAutoFollowAngularSpeed = 0.6f; // 移動方向への自動回転速度（ラジアン毎秒）
	float terrainAvoidanceAngularSpeed = 2.5f; // 地形を回り込む回転速度（ラジアン毎秒）
	float terrainAvoidanceStartRatio = 0.85f; // 通常距離に対する回避開始の割合

	// これより下はEditorに表示しない
	float currentDistance = maxDistance; // 実際にカメラが使う距離(地形で短縮される)
	float collisionRadius = 0.95f;       // カメラの太さ(壁の角へのめり込み防止)
	float collisionMargin = 0.3f;        // 壁面からの余白
	float collisionStartOffset = 5.0f; // 旧シーン互換用。近い壁を見落とすため判定始点には使用しない。
	float heightRiseFollowSpeed = 2.0f; // 高台へ上がる時の高さ追従速度。小さいほどゆっくり動く
	float heightFallFollowSpeed = 12.0f; // 下降時の高さ追従の応答速度。大きいほど早く追いつく
	float heightRiseCatchUpThreshold = 4.0f; // ジャンプ中、この高さ以上上昇したらカメラも追従する
	float lookUpCameraClearance = 1.25f; // 見上げる角度の時にプレイヤーより上へ保つカメラの高さ
	float followHeight = 0.0f; // ジャンプ中の上下移動でカメラが追いかけないための追従基準高さ
	bool followHeightInitialized = false;
	bool followHeightAirborneCatchUp = false;
	bool followHeightCatchUpInProgress = false;
	float manualControlTimer = 0.0f;
	// 回避角度だけを滑らかにし、位置全体への慣性や判定始点の移動遅れは加えない。
	float terrainAvoidanceTheta = 0.0f;
	float terrainAvoidanceHoldTimer = 0.0f;
	bool terrainAvoidanceActive = false;
	float resolvedCameraTheta = 0.0f;
	float avoidanceAngularVelocity = 0.0f;
	bool terrainYawRecovering = false;
	No::Vector3 cameraMoveVelocity{};
	No::Vector3 nominalMoveVelocity{};
	No::Vector3 collisionAnchor{};
	No::Vector3 collisionAnchorOffset{};
	bool collisionAnchorInitialized = false;
	bool collisionMotionActive = false;
	// 自動旋回中に同じ移動入力のワールド方向が変わらないよう、入力開始時のカメラ基準を保持する。
	No::Quaternion movementReferenceRotation = No::Quaternion::IDENTITY;
	No::Vector3 movementReferenceAxis{};
	bool movementReferenceLocked = false;
	No::Quaternion nextRotation = No::Quaternion::IDENTITY; // 見たい方向の回転。滑らかに角度を変えるための変数
};

inline No::Quaternion GetFollowCameraRotation(No::Registry& registry) {
	No::Quaternion rotation = No::Quaternion::IDENTITY;
	for (auto entity : registry.View<FollowCameraComponent, No::TransformComponent>()) {
		rotation = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}
	return rotation;
}

inline No::Quaternion GetPlayerMovementCameraRotation(No::Registry& registry, const No::Vector3& moveAxis) {
	No::Quaternion rotation = No::Quaternion::IDENTITY;
	const No::Vector3 horizontal{ moveAxis.x, 0.0f, moveAxis.z };
	const float length = horizontal.Length();
	for (auto entity : registry.View<FollowCameraComponent, No::TransformComponent>()) {
		auto* follow = registry.GetComponent<FollowCameraComponent>(entity);
		rotation = registry.GetComponent<No::TransformComponent>(entity)->rotation;
		if (!follow->autoFollowEnabled || follow->manualControlTimer > 0.0f || length <= 0.0001f) {
			follow->movementReferenceLocked = false;
			continue;
		}
		const No::Vector3 direction = horizontal / length;
		if (!follow->movementReferenceLocked || direction.Dot(follow->movementReferenceAxis) < 0.995f) {
			// 入力を離す、倒す方向を変える、手動でカメラを操作した場合は現在の向きから再開する。
			follow->movementReferenceRotation = rotation;
			follow->movementReferenceAxis = direction;
			follow->movementReferenceLocked = true;
		}
		rotation = follow->movementReferenceRotation;
	}
	return rotation;
}
