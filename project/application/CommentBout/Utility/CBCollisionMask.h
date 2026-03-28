#pragma once
#include "application/CommentBout/Collision/Utility/CollisionMask.h"

namespace CommentBout {

	// 互換維持用:
	// 段階移行中は旧名前空間を残しつつ、実体は CollisionMask 側の統一プリセットを参照する。
	namespace CollisionLayer {
		constexpr CommentBoutCollision::CollisionType CBPlayer = CommentBoutCollision::LayerPreset::Player;
		constexpr CommentBoutCollision::CollisionType CBPlayerAttack = CommentBoutCollision::LayerPreset::PlayerAttack;
		constexpr CommentBoutCollision::CollisionType CBGrass = CommentBoutCollision::LayerPreset::Item;
		constexpr CommentBoutCollision::CollisionType CBGround = CommentBoutCollision::LayerPreset::Ground;
		constexpr CommentBoutCollision::CollisionType CBEnemy = CommentBoutCollision::LayerPreset::Enemy;
		constexpr CommentBoutCollision::CollisionType CBEnemyBullet = CommentBoutCollision::LayerPreset::EnemyBullet;
	}

	namespace CollisionMask {
		constexpr CommentBoutCollision::CollisionType CBPlayer = CommentBoutCollision::MaskPreset::Player;
		constexpr CommentBoutCollision::CollisionType CBPlayerAttack = CommentBoutCollision::MaskPreset::PlayerAttack;
		constexpr CommentBoutCollision::CollisionType CBGrass = CommentBoutCollision::LayerPreset::PlayerAttack;
		constexpr CommentBoutCollision::CollisionType CBGround = CommentBoutCollision::MaskPreset::Ground;
		constexpr CommentBoutCollision::CollisionType CBEnemy = CommentBoutCollision::MaskPreset::Enemy;
		constexpr CommentBoutCollision::CollisionType CBEnemyBullet = CommentBoutCollision::MaskPreset::EnemyBullet;
	}

}
