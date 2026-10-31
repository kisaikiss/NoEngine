#pragma once
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Registry.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine::Gameplay {

// ノード追加UIに表示するイベントの定義。idはファイルへ保存されるため安定させる。
struct EventDescriptor {
	std::string id;
	std::string displayName;
	std::string category;
};

// Actionの編集UIとJSON検証で共有するプロパティ型。
enum class ActionPropertyType {
	Boolean,
	Integer,
	Number,
	String,
	EntityReference
};

// Action一つ分の入力欄定義。idはJSONキー、displayNameはエディタ表示名。
struct ActionPropertyDescriptor {
	std::string id;
	std::string displayName;
	ActionPropertyType type = ActionPropertyType::String;
	bool required = false;
};

// グラフ上のActionノードを実行するための登録情報。
struct ActionDescriptor {
	std::string id;
	std::string displayName;
	std::string category;
	// Action開始時に一度呼ぶ。非同期処理はイベントやコンポーネント状態を更新する。
	std::function<void(ECS::Registry&, const Event::GameplayEvent&, ECS::Entity, const nlohmann::json&)> execute;
	// trueならtarget解決を必須にする。properties内のEntityReference定義と組み合わせる。
	bool requiresTarget = true;
	std::vector<ActionPropertyDescriptor> properties;
	// 未設定ならexecuteのreturnで完了。設定時は毎フレーム確認し、trueまで次のグラフActionを待つ。
	std::function<bool(ECS::Registry&, ECS::Entity, const nlohmann::json&)> isComplete;
};

// イベント／ActionのID、表示情報、実行関数を一元管理するカタログ。
// エディタはここからノード一覧と入力欄を作り、ランタイムは同じIDから実行関数を検索する。
// エンジン共通分に加え、アプリケーションのStartupから独自イベント／Actionを登録できる。
class GameplayCatalog {
public:
	static GameplayCatalog& Get();

	bool RegisterEvent(EventDescriptor descriptor);
	bool RegisterAction(ActionDescriptor descriptor);
	bool HasEvent(const std::string& id) const;
	const EventDescriptor* FindEvent(const std::string& id) const;
	const ActionDescriptor* FindAction(const std::string& id) const;
	const std::vector<EventDescriptor>& GetEvents() const { return events_; }
	const std::vector<ActionDescriptor>& GetActions() const { return actions_; }

private:
	GameplayCatalog();
	std::vector<EventDescriptor> events_;
	std::vector<ActionDescriptor> actions_;
	std::unordered_map<std::string, size_t> eventIndices_;
	std::unordered_map<std::string, size_t> actionIndices_;
};

// イベントIDをカタログで確認してからRegistryへ積む。未登録なら警告し、falseを返す。
bool EmitGameplayEvent(ECS::Registry& registry, ECS::Entity source,
	const std::string& eventId, ECS::Entity instigator = ECS::INVALID_ENTITY);

}
