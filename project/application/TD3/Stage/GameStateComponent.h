#pragma once

/// <summary>
/// ゲーム全体の状態を保持するコンポーネント。
/// GameManagerエンティティに1つだけ付与する。
/// 各システムがこのコンポーネントを読んで動作を判断。
/// </summary>
struct GameStateComponent {
	bool editorMode = false;	// true: エディタ操作中 / false: プレイ中
};
