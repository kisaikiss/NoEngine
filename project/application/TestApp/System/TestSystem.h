#pragma once
#include "engine/NoEngine.h"
#include "engine/Window/WindowSize.h"

class TestSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float angle_ = 0.f;

	// Helper functions
	No::Vector2 WorldToScreen(const No::Vector3& worldPos, const No::Matrix4x4& viewProjection, const NoEngine::WindowSize& windowSize);
	bool CheckCircleAABBCollision(const No::Vector2& circleCenter, float radius, const No::Vector2& rectCenter, const No::Vector2& rectHalfSize);
};

