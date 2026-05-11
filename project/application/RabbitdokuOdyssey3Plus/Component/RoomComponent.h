#pragma once
#include "engine/NoEngine.h"

struct RoomRect {
	float right;
	float left;
	float top;
	float bottom;
};

struct RoomComponent {
	RoomRect bounds;        // マップの矩形
	RoomRect followZone;    // このマップでの追従範囲（DeadZone）
	uint32_t myID = 1;		// 自分のマップID
	uint32_t neighbors[4];  // 上下左右の隣接マップのID（なければ 0）
};

namespace NoEngine {

// エディタにRoomRectのタイプを登録する
template<>
struct FieldTypeResolver<RoomRect> {
	static constexpr FieldType value = FieldType::Float4;
};

}