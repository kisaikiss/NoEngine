#pragma once
#include "externals/nlohmann/json.hpp"

namespace NoEngine::Render::EditorModel {
// シーンの他の項目を保持し、書き込み完了後にだけファイルを置換する。
inline void SaveConfigurationFile(const std::filesystem::path& path,
    const nlohmann::json& expectedDiskPasses, const nlohmann::json& configuration) {
    auto temporary = path;
    temporary += ".renderpasses." + std::to_string(GetCurrentProcessId()) + ".tmp";
    try {
        const bool existed = std::filesystem::exists(path);
        auto data = nlohmann::json::object();
        std::ifstream in(path);
        if (in) data = nlohmann::json::parse(in);
        else if (existed) throw std::runtime_error("既存のシーンファイルを開けません。");
        in.close();
        if (!data.is_object()) throw std::runtime_error("シーンデータはJSONオブジェクトである必要があります。");
        if (data.value("renderPasses", nlohmann::json()) != expectedDiskPasses)
            throw std::runtime_error("ファイル内の描画パスが外部で変更されました。保存前にファイルから再読込してください。");
        const auto original = data;
        if (!data.contains("entities")) data["entities"] = nlohmann::json::object();
        data["renderPasses"] = configuration;
        std::filesystem::create_directories(path.parent_path());
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            out.exceptions(std::ios::failbit | std::ios::badbit);
            out << data.dump(4);
            out.flush();
            out.close();
        }
        // 他のエディタ操作による更新・削除を検出し、シーン情報を巻き戻さない。
        if (std::filesystem::exists(path) != existed) throw std::runtime_error("保存中にシーンファイルが変更されました。もう一度保存してください。");
        if (existed) {
            std::ifstream current(path);
            if (nlohmann::json::parse(current) != original) throw std::runtime_error("保存中にシーンファイルが変更されました。もう一度保存してください。");
        }
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("シーンファイルを置き換えられませんでした（Windowsエラー：" + std::to_string(GetLastError()) + "）。");
    } catch (...) {
        std::error_code ec;
        std::filesystem::remove(temporary, ec);
        throw;
    }
}
}
