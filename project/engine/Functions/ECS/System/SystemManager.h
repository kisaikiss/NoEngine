#pragma once
#include "ISystem.h"
#include "externals/nlohmann/json.hpp"
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <functional>

namespace NoEngine {
namespace ECS {
class SystemManager {
public:
	SystemManager();
	using SystemFactory = std::function<std::unique_ptr<ISystem>()>;
	struct SystemRegistration {
		std::string category;
		SystemFactory factory;
	};
	static bool RegisterSystemType(const std::string& name, const std::string& category,
		std::type_index type, SystemFactory factory);

	template<typename T>
	T* AddSystem(std::unique_ptr<T> system) {
		T* ptr = system.get();
		auto type = systemTypes_.find(std::type_index(typeid(T)));
		systemNames_.push_back(type == systemTypes_.end() ? std::string(typeid(T).name()) : type->second);
		systems_.push_back(std::move(system));
		return ptr;
	}

	template<typename T>
	static bool RegisterSystemType(const std::string& name, const std::string& category) {
		return RegisterSystemType(name, category, std::type_index(typeid(T)), [] { return std::make_unique<T>(); });
	}
	bool AddSystemByName(const std::string& name);
	void RemoveSystem(size_t index);
	void MoveSystem(size_t from, size_t to);
	const std::vector<std::string>& GetSystemNames() const { return systemNames_; }

	void UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime);


	// Editing以外(Playing/Paused)ならtrue。">"を押してから"■"を押すまでの間ずっとtrue。
	static bool IsInPlayMode();
	// ">"を押した瞬間(Editing→Playing)に取得したシーンのスナップショット
	static const nlohmann::json& GetPlaySnapshot();

private:
	std::vector<std::unique_ptr<ISystem>> systems_;
	std::vector<std::string> systemNames_;
	std::unordered_map<std::string, SystemRegistration> systemFactories_;
	std::unordered_map<std::type_index, std::string> systemTypes_;
	std::string configuredSceneName_;

	void LoadPlaySnapShot(Registry& registry);
	void LoadSystemConfiguration(Registry& registry);
	void SaveSystemConfiguration(Registry& registry);
};
}
}

// Register a default-constructible ISystem in the editor's categorized system list.
#define NOENGINE_REGISTER_SYSTEM_IMPL(Type, Name, Category, Line) \
	namespace { const bool noEngineSystemRegistered##Line = \
		::NoEngine::ECS::SystemManager::RegisterSystemType<Type>(Name, Category); }
#define NOENGINE_REGISTER_SYSTEM_EXPAND(Type, Name, Category, Line) \
	NOENGINE_REGISTER_SYSTEM_IMPL(Type, Name, Category, Line)
#define REGISTER_SYSTEM(Type, Name, Category) \
	NOENGINE_REGISTER_SYSTEM_EXPAND(Type, Name, Category, __LINE__)
