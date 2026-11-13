#pragma once
#include "ISystem.h"
#include "SystemCatalog.h"
#include "engine/Editor/Panels/SystemListPanel.h"
#include "engine/Editor/Panels/GameControllerPanel.h"

#include <typeindex>

namespace NoEngine {
namespace ECS {

/// <summary>
/// シーンが持つ実行対象システムのリストを保持し、UpdateAllで順に更新する。
/// カタログ・JSON永続化・再生状態・UI はそれぞれ SystemCatalog / SystemSceneConfig /
/// PlayModeController / 各Panel が担当する。
/// </summary>
class SystemManager {
	struct Entry {
		std::string name;
		std::unique_ptr<ISystem> system;
		SystemCatalog::SystemCloner cloner = {};
	};
public:
	SystemManager() = default;

	class Snapshot {
		friend class SystemManager;
		std::vector<Entry> entries_;
		Editor::SystemListPanel systemListPanel_;
		bool configurationDirty_ = false;
	};
	std::unique_ptr<Snapshot> CaptureSnapshot() const;
	void RestoreSnapshot(std::unique_ptr<Snapshot> snapshot, Registry& registry);

	template<typename T>
	T* AddSystem(std::unique_ptr<T> system) {
		T* ptr = system.get();
		std::string name = SystemCatalog::Get().NameOf(std::type_index(typeid(T)));
		// 未登録型はtypeid名で保持する。この名前はLoad時に未登録扱いとなるため、
		// 保存したJSONが読み込まれないことがある(REGISTER_SYSTEMの追加を推奨)。
		if (name.empty()) name = typeid(T).name();
		entries_.push_back({ std::move(name), std::move(system), CloneSystemForPlayMode<T> });
		ResetEditorState();
		return ptr;
	}

	bool AddSystemByName(const std::string& name);
	void RemoveSystem(size_t index);
	void MoveSystem(size_t from, size_t to);

	/// <summary>エディター操作を共通のUndo/Redo履歴に登録する。</summary>
	bool EditAddSystem(const std::string& name, size_t insertionIndex);
	bool EditRemoveSystem(size_t index);
	bool EditMoveSystem(size_t from, size_t to);

	bool HasSystem(const std::string& name) const;
	std::vector<std::string> GetSystemNames() const;

	void UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime);

	/// <summary>全システムを生成できた場合だけ構成を入れ替える。</summary>
	bool SetSystems(const std::vector<std::string>& names);
	void SaveConfiguration(Registry& registry);

private:
	class EditCommand;
	void ResetEditorState();
	void MoveEntry(size_t from, size_t to);
	std::vector<Entry> entries_;

	Editor::GameControllerPanel gameControllerPanel_;
	Editor::SystemListPanel systemListPanel_;
	bool configurationDirty_ = false;
	// シーンの構成変更や破棄後に、古い履歴がマネージャーを参照しないための寿命管理。
	std::shared_ptr<int> editLifetime_ = std::make_shared<int>(0);
};

}
}
