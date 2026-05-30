#pragma once
#include "engine/NoEngine.h"
#include "RabbitdokuSerializer.h"
struct BlockTag{};

struct GimmickTag{};

struct SaveTag{};

struct EnemyTag{};

struct RoomTag : public No::AABBCollider2D{};

struct TransitionInTag { float t = 0.0f; };

struct SceneTransitionInEvent { std::string stageName; SaveData saveData; };

struct TransitionOutTag { float t = 0.0f; };

struct SceneTransitionOutEvent {};

struct SmokeEffectTag{};

struct BackgroundTag{};
struct MainBackgroundTag{};

struct SoftlyMoveComponent {
	float t = 0.0f;
	float moveSpeed = 1.0f;
	No::Vector2 amplitude = No::Vector2::ZERO;
};

struct SpringComponent {
	float force = 900.f;
};

struct CollapseBlockComponent {
	bool startCollapse = false;
	bool startBreak = false;
	float collapseTime = 0.5f;
};

struct ReplenisherTag {};

struct DoorComponent {
	std::string stageName = "Title";
};

struct ClearItemComponent{};

struct ClearTag { 
	float t = 0;
	bool isAnimationEnd = false;
};