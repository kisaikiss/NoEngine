#pragma once

/// <summary>
/// 弾薬アイテムコンポーネント
/// 交差点に配置される弾薬アイテム
/// </summary>
struct AmmoItemComponent {
	int gridX;				// グリッド座標X
	int gridY;				// グリッド座標Y
	bool canPickup;			// 回収可能フラグ
	int ammoAmount;			// 回復する弾数

	AmmoItemComponent()
		: gridX(0),
		gridY(0),
		canPickup(false),
		ammoAmount(1) {
	}
};
