#pragma once

struct BallTag {};
struct RingTag {};
struct VausTag {};
struct EnemyTag {};

//Enemy
struct NormalEnemyTag{};
struct Boss1Tag{};
struct Boss2Tag{};
struct BatTag{};
struct BatGreenTag{};
struct CarrotTag{};
struct WhiteRadishTag{};

//Human
struct BatGirlTag{};
struct ChefTag {};
struct PlayerGirlTag {};
struct EnemyHumanTag{};

struct DeathFlag
{
	bool isDead = false;
};
struct StatusSpriteTag {};

struct TutorialSpriteTag{};