#pragma once
#include "engine/NoEngine.h"

struct RoomRect {
	float right;
	float left;
	float top;
	float bottom;

	bool Contains(No::Vector2 position) {
		if (left < position.x && position.x < right) {
			if (top < position.y && position.y < bottom) {
				return true;
			}
		}


		return false;
	}
};

enum class RoomDirection {
	RIGHT,
	LEFT,
	TOP,
	BOTTOM
};

struct RoomComponent {
	RoomRect bounds;										// マップの矩形
	RoomRect followZone;									// このマップでの追従範囲（DeadZone）
	uint32_t myID = 1;										// 自分のマップID
	std::unordered_map<RoomDirection, uint32_t> neighbor;	// 上下左右の隣接マップのID（なければ 0）
};

namespace NoEngine {

// エディタにRoomRectのタイプを登録する
template<>
struct FieldTypeResolver<RoomRect> {
	static constexpr FieldType value = FieldType::Float4;
};

}