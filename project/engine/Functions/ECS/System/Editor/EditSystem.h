#pragma once
#include "../ISystem.h"
#include "engine/Editor/HierarchyPanel.h"
#include "engine/Editor/InspectorPanel.h"
#include "engine/Editor/PrefabEditorPanel.h"
#include "engine/Editor/EntityClipboard.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace ECS {

// エディタのトップレベルシステム。各ウィンドウ（Hierarchy/Inspector/PrefabEditor）の
// パネルクラスを所有して毎フレームDrawを呼び出し、シーンの保存/読込とグローバルな
// ショートカット（Delete / Ctrl+C / Ctrl+V）だけをここで扱う。
// 新しいウィンドウ・機能を足すときは、専用のパネルクラスを engine/Editor/ 以下に追加して
// ここでインスタンス化する形にする（EditSystem自体を太らせない）。
class EditSystem :
    public ISystem {
public:
    EditSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;

private:
    Editor::HierarchyPanel hierarchyPanel_;
    Editor::InspectorPanel inspectorPanel_;
    Editor::PrefabEditorPanel prefabEditorPanel_;
    Editor::EntityClipboard clipboard_;

    // コピーやペーストを毎フレーム呼ばないためのインターバル計算用変数
    float timeInterval_ = 0.0f;

    bool FirstLoaded_ = false;


    void SaveFile(Registry& registry, nlohmann::json j);
    void LoadFile(Registry& registry);
    void EnsureUniqueEditTagNames(Registry& registry);

    void DrawMenuBar(Registry& registry);
    void HandleGlobalShortcuts(Registry& registry, float deltaTime);

    Entity SelectedNow(Registry& registry);
};

}
}