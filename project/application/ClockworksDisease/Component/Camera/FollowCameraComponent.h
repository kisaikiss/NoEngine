#pragma once
#include "engine/NoEngine.h"

struct FollowCameraComponent {
	float distance = 15.f;
	float theta = 0.f;
	float phi = 1.f;

	float idleTimer = 0.f;
	float autoFollowDelay = 1.0f;   // 何秒操作がなかったら自動追従を始めるか
	float maxAutoFollowAngularSpeed = 1.5f; // rad/sec 上限(急回転防止)
	float autoFollowDeadzoneAngle = 0.15f;  // これ未満の角度差は無視(約8.6度)

	No::Vector3 smoothedMoveDir = No::Vector3::FORWARD; // 平滑化された移動方向
	No::Vector3 prevPlayerPos{};
	bool hasPrevPlayerPos = false;
};