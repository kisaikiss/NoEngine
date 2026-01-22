#pragma once
#include "engine/Math/MathInclude.h"

struct PhysicsComponent
{ 
    NoEngine::Vector3 velocity;
	float coefficient = 0.8f;				//反発係数
	bool useGravity = true;					//重力を使うか

	static constexpr float kGravity = 9.8f;	//重力
};