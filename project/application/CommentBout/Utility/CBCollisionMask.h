#pragma once
#include "application/TestApp/Utility/CollisionMask.h"

namespace CommentBout {

	namespace CollisionLayer {
		constexpr TestApp::CollisionType CBPlayer = TestApp::CollisionType::Player;
		constexpr TestApp::CollisionType CBPlayerAttack = TestApp::CollisionType::PlayerBullet;
		constexpr TestApp::CollisionType CBGrass = TestApp::CollisionType::Item;
		constexpr TestApp::CollisionType CBGround = TestApp::CollisionType::Block;
		constexpr TestApp::CollisionType CBEnemy = TestApp::CollisionType::Enemy;
		constexpr TestApp::CollisionType CBEnemyBullet = TestApp::CollisionType::EnemyBullet;
	}

	namespace CollisionMask {
		constexpr TestApp::CollisionType CBPlayer =
			CollisionLayer::CBEnemy |
			CollisionLayer::CBEnemyBullet |
			CollisionLayer::CBGround;

		constexpr TestApp::CollisionType CBPlayerAttack =
			CollisionLayer::CBGrass |
			CollisionLayer::CBEnemy;

		constexpr TestApp::CollisionType CBGrass =
			CollisionLayer::CBPlayerAttack;

		constexpr TestApp::CollisionType CBGround =
			TestApp::CollisionType::None;

		constexpr TestApp::CollisionType CBEnemy =
			CollisionLayer::CBPlayer |
			CollisionLayer::CBPlayerAttack;


		constexpr TestApp::CollisionType CBEnemyBullet =
			CollisionLayer::CBPlayer |
			CollisionLayer::CBGround;
	}

}
