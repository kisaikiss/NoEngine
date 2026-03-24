#pragma once
#include "application/CommentBout/Collision/Utility/CollisionMask.h"

namespace CommentBout {

	namespace CollisionLayer {
		constexpr CommentBoutCollision::CollisionType CBPlayer = CommentBoutCollision::CollisionType::Player;
		constexpr CommentBoutCollision::CollisionType CBPlayerAttack = CommentBoutCollision::CollisionType::PlayerBullet;
		constexpr CommentBoutCollision::CollisionType CBGrass = CommentBoutCollision::CollisionType::Item;
		constexpr CommentBoutCollision::CollisionType CBGround = CommentBoutCollision::CollisionType::Block;
		constexpr CommentBoutCollision::CollisionType CBEnemy = CommentBoutCollision::CollisionType::Enemy;
		constexpr CommentBoutCollision::CollisionType CBEnemyBullet = CommentBoutCollision::CollisionType::EnemyBullet;
	}

	namespace CollisionMask {
		constexpr CommentBoutCollision::CollisionType CBPlayer =
			CollisionLayer::CBEnemy |
			CollisionLayer::CBEnemyBullet |
			CollisionLayer::CBGround;

		constexpr CommentBoutCollision::CollisionType CBPlayerAttack =
			CollisionLayer::CBGrass |
			CollisionLayer::CBEnemy;

		constexpr CommentBoutCollision::CollisionType CBGrass =
			CollisionLayer::CBPlayerAttack;

		constexpr CommentBoutCollision::CollisionType CBGround =
			CommentBoutCollision::CollisionType::None;

		constexpr CommentBoutCollision::CollisionType CBEnemy =
			CollisionLayer::CBPlayer |
			CollisionLayer::CBPlayerAttack;


		constexpr CommentBoutCollision::CollisionType CBEnemyBullet =
			CollisionLayer::CBPlayer |
			CollisionLayer::CBGround;
	}

}
