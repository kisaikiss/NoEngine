#pragma once

struct BallTag {};
struct RingTag {};
struct VausTag {};
struct EnemyTag {};

//Enemy
struct NormalEnemyTag{};
struct Boss1Tag{};

//Human
struct BatGirlTag{};
struct PlayerGirlTag {};

struct DeathFlag
{
	bool isDead = false;
};