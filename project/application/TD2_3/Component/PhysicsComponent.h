#pragma once
#include "engine/Math/MathInclude.h"

struct PhysicsComponent
{ 
    NoEngine::Vector3 velocity;
	float coefficient = 0.8f;				//反発係数
	float friction = 0.3f;					//摩擦係数
	bool useGravity = true;					//重力を使うか
	float baseSpeed = 5.0f;					//基本速度
	static constexpr float kGravity = 6.0f;	//重力
};