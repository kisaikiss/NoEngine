#pragma once

// プレイヤー関連
struct PlayerTag {};
struct PlayerBulletTag {};
struct ShockwaveTag {};

// 敵関連
struct EnemyTag {};

// アイテム関連
struct AmmoItemTag {};

// 共通フラグ
struct DeathFlag {
	bool isDead = false;
};