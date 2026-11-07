#pragma once
#include "externals/nlohmann/json.hpp"

namespace NoEngine::Render::EditorModel {
using Json = nlohmann::json;
enum class ResourceType { Color, Depth };
struct Resource {
    std::string name;
    ResourceType type = ResourceType::Color;
    unsigned format = 0;
    unsigned arraySize = 1;
    bool uav = false;
};
struct Pass {
    std::string name;
    Json defaults;
    std::unordered_map<std::string, ResourceType> inputTypes;
    std::vector<std::string> prerequisites;
    bool visible = true;
};
struct Diagnostic {
    std::string pass;
    std::string message;
};
struct Document {
    Json draft = Json::array();
    Json saved = Json::array();
    Json applied = Json::array();
    // ディスク上の原文は、既定値を補った編集用データと区別して競合を検出する。
    Json diskPasses;
    Json pendingBefore;
    std::string selected;
    std::string status;
};

inline const Resource* FindResource(const std::vector<Resource>& resources, const std::string& name) {
    const auto it = std::find_if(resources.begin(), resources.end(), [&](const Resource& r) { return r.name == name; });
    return it == resources.end() ? nullptr : &*it;
}

// PSOの出力形式とシェーダーのテクスチャ次元を保つため、既定スロットと互換な候補だけを許可する。
inline bool Compatible(const std::vector<Resource>& resources, const std::string& expected,
    const std::string& candidate, ResourceType type, bool uav = false) {
    const auto* source = FindResource(resources, expected);
    const auto* target = FindResource(resources, candidate);
    return source && target && target->type == type && source->type == type
        && source->format == target->format && source->arraySize == target->arraySize && (!uav || target->uav);
}

inline bool HasName(const Json& entry, const std::string& name) {
    return entry.is_object() && entry.contains("name") && entry["name"].is_string()
        && entry["name"].get_ref<const std::string&>() == name;
}

inline Json Normalize(const Json& entries, const std::vector<Pass>& passes) {
    if (!entries.is_array()) return entries;
    Json result = entries;
    for (auto& entry : result) {
        if (!entry.is_object() || !entry.contains("name") || !entry["name"].is_string()) continue;
        const auto found = std::find_if(passes.begin(), passes.end(), [&](const Pass& p) { return p.name == entry["name"].get_ref<const std::string&>(); });
        if (found == passes.end()) continue;
        auto settings = found->defaults;
        if (entry.contains("settings")) {
            if (!entry["settings"].is_object()) continue;
            settings.update(entry["settings"], true);
        }
        entry["settings"] = std::move(settings);
    }
    return result;
}

inline std::vector<Diagnostic> Validate(const Json& configuration, const std::vector<Pass>& passes,
    const std::vector<Resource>& resources, const std::string& finalOutput) {
    std::vector<Diagnostic> errors;
    auto error = [&](const std::string& pass, const std::string& message) { errors.push_back({pass, message}); };
    if (!configuration.is_array()) { error("", "パスの構成はJSON配列である必要があります。"); return errors; }
    Json entries = configuration;
    for (const auto& pass : passes) if (!pass.visible) entries.push_back({{"name", pass.name}, {"settings", pass.defaults}});
    std::unordered_set<std::string> seen, written;
    bool mainOutputWritten = false;
    for (size_t index = 0; index < entries.size(); ++index) {
        // デバッグ描画の出力でゲーム画面の欠落を隠さない。
        if (index == configuration.size()) mainOutputWritten = written.contains(finalOutput);
        const auto& entry = entries[index];
        if (!entry.is_object() || !entry.contains("name") || !entry["name"].is_string()) {
            error("", "各パスには文字列の名前が必要です。"); continue;
        }
        const std::string name = entry["name"];
        const auto schema = std::find_if(passes.begin(), passes.end(), [&](const Pass& p) { return p.name == name; });
        if (schema == passes.end() || (index < configuration.size() && !schema->visible)) {
            error(name, "未登録のパス、またはエディタ専用のパスです。"); continue;
        }
        if (seen.contains(name)) { error(name, "同じパスが重複しています。"); continue; }
        const size_t errorsBefore = errors.size();
        for (const auto& required : schema->prerequisites)
            if (!seen.contains(required)) error(name, "先に実行する必要があるパス：" + required);
        if (!entry.contains("settings") || !entry["settings"].is_object()) {
            error(name, "設定データの形式が不正です。"); continue;
        }
        const auto& settings = entry["settings"];
        // 数値の変換例外や不正なJSON形状を、実行用パスに渡す前に検出する。
        for (auto it = schema->defaults.begin(); it != schema->defaults.end(); ++it) {
            if (!settings.contains(it.key())) { error(name, "設定項目がありません：" + it.key()); continue; }
            const auto& value = settings[it.key()];
            if (it.value().is_number()) {
                if (!value.is_number() || !std::isfinite(value.get<double>()) || std::abs(value.get<double>()) > 1.0e30)
                    error(name, "数値が不正です：" + it.key());
                else if (it.value().is_number_integer() && (!value.is_number_integer() || value.get<double>() < -2147483648.0 || value.get<double>() > 2147483647.0))
                    error(name, "整数値が不正です：" + it.key());
            } else if (value.type() != it.value().type()) error(name, "設定値の型が不正です：" + it.key());
        }
        if (errors.size() != errorsBefore) continue;
        const auto& defaults = schema->defaults;
        if (settings["targetCameraType"].get<int>() < 0 || settings["targetCameraType"].get<int>() > 2)
            error(name, "描画カメラの種類が不正です。");
        std::unordered_set<std::string> reads, writes;
        const auto& inputs = settings["inputs"];
        if (inputs.size() != defaults["inputs"].size()) error(name, "入力スロットがパスの定義と一致しません。");
        for (auto it = defaults["inputs"].begin(); it != defaults["inputs"].end(); ++it) {
            if (!inputs.contains(it.key()) || !inputs[it.key()].is_string()) { error(name, "入力がありません：" + it.key()); continue; }
            const std::string resource = inputs[it.key()];
            const auto type = schema->inputTypes.find(it.key());
            if (type == schema->inputTypes.end() || !Compatible(resources, it.value().get<std::string>(), resource, type->second))
                error(name, "互換性のない入力です：" + it.key() + ": " + resource);
            if (!written.contains(resource)) error(name, "入力「" + resource + "」を生成するパスが先にありません。生成元のパスを前に移動するか追加してください。");
            reads.insert(resource);
        }
        for (const char* key : {"outputs", "uavOutputs"}) {
            if (settings[key].size() != defaults[key].size()) { error(name, std::string("スロット数が不正です：") + key); continue; }
            for (size_t i = 0; i < settings[key].size(); ++i) {
                if (!settings[key][i].is_string()) { error(name, "出力名は文字列である必要があります。"); continue; }
                const std::string resource = settings[key][i];
                if (!Compatible(resources, defaults[key][i].get<std::string>(), resource, ResourceType::Color, std::string(key) == "uavOutputs"))
                    error(name, "互換性のない出力です：" + resource);
                if (std::string(key) == "outputs" && inputs.empty() && !settings["autoClear"].get<bool>() && !written.contains(resource))
                    error(name, "カラー出力をクリアするか、先行するパスで初期化してください：" + resource);
                if (!writes.insert(resource).second) error(name, "出力が重複しています：" + resource);
            }
        }
        const std::string depth = settings["depthOutput"];
        const std::string defaultDepth = defaults["depthOutput"];
        if (defaultDepth.empty() != depth.empty() || (!depth.empty() && !Compatible(resources, defaultDepth, depth, ResourceType::Depth)))
            error(name, "互換性のない深度出力です：" + depth);
        if (!depth.empty()) {
            if (!settings["clearDepth"].get<bool>() && !written.contains(depth)) error(name, "深度出力が先行するパスで初期化されていません：" + depth);
            writes.insert(depth);
        }
        for (const auto& resource : reads) if (writes.contains(resource)) error(name, "同じパス内で同一リソースを読み書きすることはできません：" + resource);
        if (errors.size() == errorsBefore) {
            written.insert(writes.begin(), writes.end());
            seen.insert(name);
        }
    }
    if (entries.size() == configuration.size()) mainOutputWritten = written.contains(finalOutput);
    if (!mainOutputWritten) error("", "ゲーム画面の出力「" + finalOutput + "」を生成する有効なパスがありません。");
    return errors;
}
}
