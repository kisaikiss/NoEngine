#pragma once
#include "engine/NoEngine.h"

struct RabbitdokuPushBackEvent {
	No::Entity player;
	No::ContactPosition position;
};

struct RabbitdokuItemGetEvent {
	No::Entity player;
	No::Entity item;
};

struct RabbitdokuCollisionLayerComponent {
	enum CollisionLayer : uint32_t {
		None = 0,
		Player = 1 << 0,	// プレイヤー
		Enemy = 1 << 1,		// 敵
		Terrain = 1 << 2,	// 地形
		Item = 1 << 3,		// アイテム
	};

	CollisionLayer layer;
	
};

namespace NoEngine {

// エディタにCollisionLayerのタイプを登録する
template<>
struct FieldTypeResolver<RabbitdokuCollisionLayerComponent::CollisionLayer> {
	static constexpr FieldType value = FieldType::Int;
};

}