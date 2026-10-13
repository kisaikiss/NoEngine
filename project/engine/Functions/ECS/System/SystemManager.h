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
public:
	SystemManager() = default;

	template<typename T>
	T* AddSystem(std::unique_ptr<T> system) {
		T* ptr = system.get();
		std::string name = SystemCatalog::Get().NameOf(std::type_index(typeid(T)));
		// 未登録型はtypeid名で保持する。この名前はLoad時に未登録扱いとなるため、
		// 保存したJSONが読み込まれないことがある(REGISTER_SYSTEMの追加を推奨)。
		if (name.empty()) name = typeid(T).name();
		entries_.push_back({ std::move(name), std::move(system) });
		return ptr;
	}

	bool AddSystemByName(const std::string& name);
	void RemoveSystem(size_t index);
	void MoveSystem(size_t from, size_t to);

	bool HasSystem(const std::string& name) const;
	std::vector<std::string> GetSystemNames() const;

	void UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime);

	/// <summary>現在のシーン名に対応するJSONの"systems"を適用する(シーンごとに1回だけ)</summary>
	void LoadConfiguration(Registry& registry);
	void SaveConfiguration(Registry& registry);

private:
	struct Entry {
		std::string name;
		std::unique_ptr<ISystem> system;
	};
	std::vector<Entry> entries_;
	std::string configuredSceneName_;

	Editor::GameControllerPanel gameControllerPanel_;
	Editor::SystemListPanel systemListPanel_;
};

}
}