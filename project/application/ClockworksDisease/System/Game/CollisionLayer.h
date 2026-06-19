#pragma once
#include "engine/NoEngine.h"

struct CollisionLayerComponent {
	enum CollisionLayer : uint32_t {
		None = 0,
		Player = 1 << 0,	// プレイヤー
		Enemy = 1 << 1,		// 敵
		Terrain = 1 << 2,	// 地形
		Item = 1 << 3,		// アイテム
	};

	CollisionLayer layer;
};

// コリジョンレイヤータイプをUint型としてエディタに登録する
namespace NoEngine {
template<>
struct FieldTypeResolver<CollisionLayerComponent::CollisionLayer> {
	static constexpr FieldType value = FieldType::Uint;
};
}