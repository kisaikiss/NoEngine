#pragma once

/// <summary>
/// ダメージ無効時間を管理する共通コンポーネント。
/// 現時点では主にプレイヤーに付与して、被弾直後の無敵を制御する。
/// </summary>
struct InvincibleComponent {
	float time = 0.0f;
	float duration = 0.35f;
};
