#pragma once


/// <summary>
/// グリッドセルコンポーネント
/// グリッド上のノード（交差点）の接続情報を保持
/// </summary>
struct GridCellComponent {
	int gridX;  // グリッド座標X
	int gridY;  // グリッド座標Y

	// 4方向の接続情報
	bool hasConnectionUp;
	bool hasConnectionRight;
	bool hasConnectionDown;
	bool hasConnectionLeft;

	// 敵専用道フラグ（true のときプレイヤーは侵入不可）
	bool isEnemyOnly;

	GridCellComponent()
		: gridX(0), gridY(0),
		hasConnectionUp(false),
		hasConnectionRight(false),
		hasConnectionDown(false),
		hasConnectionLeft(false),
		isEnemyOnly(false) {
	}
};