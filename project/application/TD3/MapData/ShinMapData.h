#pragma once
#include "../Component/GridCellComponent.h"

namespace MapData {

	// マップセルデータ構造体
	struct CellData {
		int x, y;
		bool up, right, down, left;
	};

	// 申の字マップ定義（2倍サイズ版）
	//
	// 元マップの全座標を ×2 し、接続している各ノード間に中間ノードを追加することで
	// 物理的に2倍の大きさにしている。
	// 元11ノード → 中間ノード14個追加 → 合計25ノード
	//
	//           (2,6)               ← 最上端（行き止まり）
	//             |
	//           (2,5)               ← 中間ノード
	//             |
	// (0,4)-(1,4)-(2,4)-(3,4)-(4,4)   ← 上段
	//   |           |           |
	// (0,3)       (2,3)       (4,3)   ← 中間ノード（縦）
	//   |           |           |
	// (0,2)-(1,2)-(2,2)-(3,2)-(4,2)   ← 中段（(2,2) が初期位置）
	//   |           |           |
	// (0,1)       (2,1)       (4,1)   ← 中間ノード（縦）
	//   |           |           |
	// (0,0)-(1,0)-(2,0)-(3,0)-(4,0)   ← 下段
	//               |
	//             (2,-1)          ← 中間ノード
	//               |
	//             (2,-2)          ← 最下端（行き止まり）
	//
	// 合計25ノード
	//
	const CellData SHIN_MAP[] = {

		// ========== 元のノード（座標 ×2） ==========

		// 最上端（下接続のみ）- 行き止まり
		{2,  6, false, false, true,  false},

		// 上段
		{0,  4, false, true,  true,  false},  // 左上 - L字
		{2,  4, true,  true,  true,  true },  // 中上 - 十字
		{4,  4, false, false, true,  true },  // 右上 - L字

		// 中段
		{0,  2, true,  true,  true,  false},  // 左中 - T字
		{2,  2, true,  true,  true,  true },  // 中央 - 十字（初期位置）
		{4,  2, true,  false, true,  true },  // 右中 - T字

		// 下段
		{0,  0, true,  true,  false, false},  // 左下 - L字
		{2,  0, true,  true,  true,  true },  // 中下 - 十字
		{4,  0, true,  false, false, true },  // 右下 - L字

		// 最下端（上接続のみ）- 行き止まり
		{2, -2, true,  false, false, false},

		// ========== 中間ノード（各エッジの中点） ==========

		// 縦エッジの中間ノード
		{2,  5, true,  false, true,  false},  // (2,6)  ↔ (2,4)  の中間
		{0,  3, true,  false, true,  false},  // (0,4)  ↔ (0,2)  の中間
		{2,  3, true,  false, true,  false},  // (2,4)  ↔ (2,2)  の中間
		{4,  3, true,  false, true,  false},  // (4,4)  ↔ (4,2)  の中間
		{0,  1, true,  false, true,  false},  // (0,2)  ↔ (0,0)  の中間
		{2,  1, true,  false, true,  false},  // (2,2)  ↔ (2,0)  の中間
		{4,  1, true,  false, true,  false},  // (4,2)  ↔ (4,0)  の中間
		{2, -1, true,  false, true,  false},  // (2,0)  ↔ (2,-2) の中間

		// 横エッジの中間ノード
		{1,  4, false, true,  false, true },  // (0,4)  ↔ (2,4)  の中間
		{3,  4, false, true,  false, true },  // (2,4)  ↔ (4,4)  の中間
		{1,  2, false, true,  false, true },  // (0,2)  ↔ (2,2)  の中間
		{3,  2, false, true,  false, true },  // (2,2)  ↔ (4,2)  の中間
		{1,  0, false, true,  false, true },  // (0,0)  ↔ (2,0)  の中間
		{3,  0, false, true,  false, true },  // (2,0)  ↔ (4,0)  の中間
	};

	const size_t SHIN_MAP_SIZE = sizeof(SHIN_MAP) / sizeof(CellData);

} // namespace MapData