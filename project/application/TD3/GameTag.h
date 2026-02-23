#pragma once

// プレイヤー関連
struct PlayerTag {};
struct PlayerBulletTag {};

// アイテム関連
struct AmmoItemTag {};

// 共通フラグ
struct DeathFlag {
	bool isDead = false;
};
