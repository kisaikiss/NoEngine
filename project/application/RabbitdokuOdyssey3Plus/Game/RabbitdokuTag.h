#pragma once
#include "engine/NoEngine.h"
struct BlockTag{};

struct GimmickTag{};

struct SaveTag{};

struct EnemyTag{};

struct RoomTag : public No::AABBCollider2D{};

struct TransitionInTag { float t = 0.0f; };

struct SceneTransitionInEvent{};

struct TransitionOutTag { float t = 0.0f; };

struct SceneTransitionOutEvent {};

struct SmokeEffectTag{};