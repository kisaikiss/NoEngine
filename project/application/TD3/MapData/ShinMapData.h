#pragma once
#include "../Component/GridCellComponent.h"

namespace MapData {

	// マップセルデータ構造体
	struct CellData {
		int x, y;
		bool up, right, down, left;
	};

	// 申の字マップ定義
	//
	//       (1,3)
	//         |
	// (0,2)-(1,2)-(2,2)
	//   |     |     |
	// (0,1)-(1,1)-(2,1)
	//   |     |     |
	// (0,0)-(1,0)-(2,0)
	//         |
	//       (1,-1)
	//
	// 合計11ノード
	//
	const CellData SHIN_MAP[] = {
		// 最上端（下接続のみ）- 行き止まり
		{1, 3, false, false, true, false},
		
		// 上段
		{0, 2, false, true, true, false},   // 左上 - L字
		{1, 2, true, true, true, true},     // 中上 - 十字
		{2, 2, false, false, true, true},   // 右上 - L字
		
		// 中段
		{0, 1, true, true, true, false},    // 左中 - T字
		{1, 1, true, true, true, true},     // 中央 - 十字（初期位置）
		{2, 1, true, false, true, true},    // 右中 - T字
		
		// 下段
		{0, 0, true, true, false, false},   // 左下 - L字
		{1, 0, true, true, true, true},     // 中下 - 十字
		{2, 0, true, false, false, true},   // 右下 - L字
		
		// 最下端（上接続のみ）- 行き止まり
		{1, -1, true, false, false, false}
	};

	const size_t SHIN_MAP_SIZE = sizeof(SHIN_MAP) / sizeof(CellData);

} // namespace MapData
