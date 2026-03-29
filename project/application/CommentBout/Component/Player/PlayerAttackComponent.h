#pragma once
#include "engine/NoEngine.h"
#include "engine/Editor/ReflectionMacros.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"

struct PlayerAttackComponent {
	No::Vector2 spawnOffset{ 0.0f, -80.0f };
	No::Vector2 attackSize{ 140.0f, 140.0f };
	float visibleTime = 0.35f;
	int attackLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PlayerAttack));
	int attackPower = 10;
};
