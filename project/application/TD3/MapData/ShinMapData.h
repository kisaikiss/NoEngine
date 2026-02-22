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
	//      (1,2)
	//        |
	// (0,1)-(1,1)-(2,1)
	//        |
	//      (1,0)
	//
	const CellData SHIN_MAP[] = {
		// 左端（右接続のみ）
		{0, 1, false, true, false, false},

		// 下端（上接続のみ）
		{1, 0, true, false, false, false},

		// 中央（十字、全方向接続）
		{1, 1, true, true, true, true},

		// 上端（下接続のみ）
		{1, 2, false, false, true, false},

		// 右端（左接続のみ）
		{2, 1, false, false, false, true}
	};

	const size_t SHIN_MAP_SIZE = sizeof(SHIN_MAP) / sizeof(CellData);

} // namespace MapData
