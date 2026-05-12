#pragma once
#include "engine/NoEngine.h"

struct RoomRect {
	float right;
	float left;
	float top;
	float bottom;

	bool Contains(const No::Vector2& position) {
		if (left < position.x && position.x < right) {
			if (top < position.y && position.y < bottom) {
				return true;
			}
		}
		return false;
	}

	No::Vector2 Center() {
		No::Vector2 result{};
		result.x = (left + right) / 2.f;
		result.y = (top + bottom) / 2.f;
		return result;
	}
};

enum class RoomDirection {
	RIGHT,
	LEFT,
	TOP,
	BOTTOM
};

struct RoomComponent {
	RoomRect bounds{};										// マップの矩形
	uint32_t myID = 1;										// 自分のマップID
};

namespace NoEngine {

// エディタにRoomRectのタイプを登録する
template<>
struct FieldTypeResolver<RoomRect> {
	static constexpr FieldType value = FieldType::Float4;
};

}