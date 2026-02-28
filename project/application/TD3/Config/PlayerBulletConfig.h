#pragma once

/// <summary>
/// プレイヤーの弾丸のグローバル設定
/// ImGuiで調整可能
/// </summary>
struct PlayerBulletConfig {
	// デフォルトの弾の挙動フラグ
	bool defaultPenetrateWalls = true;			// 壁を貫通するか
	bool defaultPenetrateEnemies = true;		// 敵を貫通するか
	bool defaultEnableLooping = true;			// 画面外ループを有効にするか
	bool defaultDisableLoopOnHit = true;		// 敵に当たった時にループを無効化するか
	float defaultScreenBoundsOffset = 0.5f;		// 画面外判定のオフセット

	static PlayerBulletConfig& GetInstance() {
		static PlayerBulletConfig instance;
		return instance;
	}

private:
	PlayerBulletConfig() = default;
};
