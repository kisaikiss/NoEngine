#pragma once

struct CollisionLayerComponent {
	enum CollisionLayer : uint32_t {
		None = 0,
		Player = 1 << 0,	// プレイヤー
		Enemy = 1 << 1,		// 敵
		Terrain = 1 << 2,	// 地形
	};

	CollisionLayer layer;
};