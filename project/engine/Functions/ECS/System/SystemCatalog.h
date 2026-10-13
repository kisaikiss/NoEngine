#pragma once
#include <typeindex>
#include "ISystem.h"

namespace NoEngine {
namespace ECS {

/// <summary>
/// REGISTER_SYSTEMで登録されたシステム型のカタログ。
/// 名前からのインスタンス生成、型から名前の逆引き、カテゴリ別一覧を提供する。
/// </summary>
class SystemCatalog {
public:
	using SystemFactory = std::function<std::unique_ptr<ISystem>()>;

	static SystemCatalog& Get();

	bool Register(const std::string& name, const std::string& category,
		std::type_index type, SystemFactory factory);

	template<typename T>
	bool Register(const std::string& name, const std::string& category) {
		return Register(name, category, std::type_index(typeid(T)),
			[] { return std::make_unique<T>(); });
	}

	/// <summary>名前からシステムを生成する。未登録ならnullptr</summary>
	std::unique_ptr<ISystem> Create(const std::string& name) const;

	bool Contains(const std::string& name) const;

	/// <summary>型から登録名を逆引きする。未登録なら空文字</summary>
	std::string NameOf(std::type_index type) const;

	/// <summary>カテゴリ名 -> 名前一覧(名前は昇順ソート済み)</summary>
	std::map<std::string, std::vector<std::string>> GroupedByCategory() const;

private:
	SystemCatalog() = default;

	struct Entry {
		std::string category;
		SystemFactory factory;
	};
	std::unordered_map<std::string, Entry> entries_;
	std::unordered_map<std::type_index, std::string> types_;
};

}
}

// デフォルトコンストラクタを持つ ISystem を、エディターのカテゴリ分けされたシステムリストに登録します。
#define NOENGINE_REGISTER_SYSTEM_IMPL(Type, Name, Category, Line) \
	namespace { const bool noEngineSystemRegistered##Line = \
		::NoEngine::ECS::SystemCatalog::Get().Register<Type>(Name, Category); }
#define NOENGINE_REGISTER_SYSTEM_EXPAND(Type, Name, Category, Line) \
	NOENGINE_REGISTER_SYSTEM_IMPL(Type, Name, Category, Line)
#define REGISTER_SYSTEM(Type, Name, Category) \
	NOENGINE_REGISTER_SYSTEM_EXPAND(Type, Name, Category, __LINE__)