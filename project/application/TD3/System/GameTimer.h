#pragma once

/// <summary>
/// ゲームタイマー
/// プレイヤーが移動中のときのみ時間が進む専用タイマー
/// 敵の移動やスポナーなどゲーム内の時間管理に使用する
/// </summary>
class GameTimer {
public:
	GameTimer()
		: gameTime_(0.0f)
		, gameDeltaTime_(0.0f)
		, timeScale_(1.0f)
		, isPaused_(false)
	{
	}

	/// <summary>
	/// タイマーを更新する
	/// プレイヤーが移動中の場合のみ時間を進める
	/// </summary>
	/// <param name="realDeltaTime">実際の経過時間</param>
	/// <param name="isPlayerMoving">プレイヤーが移動中かどうか</param>
	void Update(float realDeltaTime, bool isPlayerMoving) {
		if (isPaused_ || !isPlayerMoving) {
			gameDeltaTime_ = 0.0f;
			return;
		}

		gameDeltaTime_ = realDeltaTime * timeScale_;
		gameTime_ += gameDeltaTime_;
	}

	/// <summary>
	/// ゲーム時間の経過量（前フレームからの差分）を取得
	/// </summary>
	float GetGameDeltaTime() const { return gameDeltaTime_; }

	/// <summary>
	/// ゲーム開始からの累積時間を取得
	/// </summary>
	float GetGameTime() const { return gameTime_; }

	/// <summary>
	/// タイムスケールを設定（デバッグ用の倍速・スロー再生）
	/// </summary>
	void SetTimeScale(float scale) { timeScale_ = scale; }

	/// <summary>
	/// タイムスケールを取得
	/// </summary>
	float GetTimeScale() const { return timeScale_; }

	/// <summary>
	/// ポーズ状態を設定
	/// </summary>
	void SetPaused(bool paused) { isPaused_ = paused; }

	/// <summary>
	/// ポーズ状態を取得
	/// </summary>
	bool IsPaused() const { return isPaused_; }

	/// <summary>
	/// タイマーをリセット
	/// </summary>
	void Reset() {
		gameTime_ = 0.0f;
		gameDeltaTime_ = 0.0f;
	}

private:
	float gameTime_;			// ゲーム開始からの累積時間
	float gameDeltaTime_;		// 前フレームからの経過時間（ゲーム時間）
	float timeScale_;			// タイムスケール（1.0 = 通常速度）
	bool isPaused_;				// ポーズ中かどうか
};
