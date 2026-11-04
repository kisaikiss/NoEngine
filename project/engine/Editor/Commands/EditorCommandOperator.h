#pragma once
#include "engine/Functions/Command/CommandManager.h"

namespace NoEngine {
namespace ECS { class Registry; }
namespace Editor {
// このスコープ内のコマンドが参照する専用Registryを、履歴の寿命まで保持する。
class ScopedEditorRegistry {
public:
	explicit ScopedEditorRegistry(std::shared_ptr<ECS::Registry> registry);
	~ScopedEditorRegistry();
	ScopedEditorRegistry(const ScopedEditorRegistry&) = delete;
	ScopedEditorRegistry& operator=(const ScopedEditorRegistry&) = delete;
	static bool IsIsolated();
private:
	std::shared_ptr<ECS::Registry> previous_;
};

class EditorCommandOperator {
public:
	/// <summary>
	/// Undoスタックへコマンドの追加を行います
	/// </summary>
	/// <param name="cmd">追加するコマンド</param>
	static void AddCommand(std::unique_ptr<Command::ICommand> cmd);

	/// <summary>
	/// 毎フレーム呼ぶ関数。ctrl+zやctrl+shift+zなどでUndo, Redoを呼び出します。
	/// </summary>
	static void Update(float deltaTime);

	/// <summary>
	/// Undoスタックのコマンドを取り出し、Redoスタックへ移動します。
	/// </summary>
	static void Undo();

	/// <summary>
	/// Redoスタックのコマンドを取り出し、Undoスタックへ移動します。
	/// </summary>
	static void Redo();

	static bool CanUndo();
	static bool CanRedo();

	static void Reset();

	static void Shutdown();
private:
};
}
}
