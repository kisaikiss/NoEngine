#pragma once
#include "../ISystem.h"
#include "externals/nlohmann/json.hpp"
#include <unordered_map>
#include <vector>
#include <set>
#include <string>
namespace NoEngine {
\
namespace ECS {
class EditSystem :
    public ISystem {
public:
    EditSystem() { SetStopInGameStop(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    struct EditorState {
        Entity selectedEntity = INVALID_ENTITY;     // Inspectorに表示する代表Entity（EditSelectedTagと同期・排他）
        std::set<Entity> hierarchySelection;        // Hierarchyウィンドウ上の複数選択（ECSタグではない内部状態）
        Entity lastClickedEntity = INVALID_ENTITY;   // Shift+Click範囲選択の基準Entity
    };

    EditorState editorState_;
    // ctrl + cでコピーしたオブジェクトを一時保存する変数
    nlohmann::json copyObject_;
    // コピーやペーストを毎フレーム呼ばないためのインターバル計算用変数
    float timeInterval_ = 0.0f;

    char addComponentFilter_[128] = {};

    ECS::Entity editingPrefabEntity_ = ECS::INVALID_ENTITY;
    std::string editingPrefabPath_;

    bool FirstLoaded_ = false;

    // Hierarchy上の表示順（プリオーダー）。Shift+Click範囲選択の基準に使う。毎フレーム再構築。
    std::vector<Entity> hierarchyOrder_;

    // 複数選択済みの要素をクリックした際、ドラッグに発展するかどうかが分かるまで
    // 単一選択への還元を保留しておくためのEntity（BeginDragDropSourceが成立したらクリアする）
    ECS::Entity pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;

    void SaveFile(Registry& registry, nlohmann::json j);
    void LoadFile(Registry& registry);

    void DrawHierarchyWindow(Registry& registry);
    void DrawEntityNode(ECS::Registry& registry, ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap);
    void DrawAddComponentMenu(Registry& registry, Entity entity);
    void EnsureUniqueEditTagNames(Registry& registry);\


    // childの親をnewParentに変更する（Transform / Transform2Dの parent も型が一致する場合のみ同期する）
    void SetEntityParent(ECS::Registry& registry, ECS::Entity child, ECS::Entity newParent);
    // ancestorCandidate が entity の子孫かどうか（循環参照防止用）
    bool IsDescendantOf(ECS::Registry& registry, ECS::Entity ancestorCandidate, ECS::Entity entity);

    // --- Hierarchy上の複数選択 ---
    // クリック（単一選択 / Ctrl+Click追加解除 / Shift+Click範囲選択）を処理し、Inspector用の代表選択も更新する
    void HandleHierarchyClick(ECS::Registry& registry, ECS::Entity e);
    // 現在のツリー表示順（プリオーダー）をhierarchyOrder_に構築する
    void BuildHierarchyOrder(ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap, std::vector<ECS::Entity>& outOrder);
    // ドラッグ&ドロップで受け取ったENTITY_MULTIペイロードを newParent の子として付け替える（複数選択対応）
    void HandleHierarchyDrop(ECS::Registry& registry, ECS::Entity newParent);

    // --- 親をコピペすると子も全てコピペされるようにするための再帰処理 ---
    void CollectSubtreeEntities(ECS::Registry& registry, ECS::Entity root, std::vector<ECS::Entity>& outEntities);

    void BeginEditPrefab(ECS::Registry& registry, const std::string& prefabPath);
    void EndEditPrefab(ECS::Registry& registry);

};
}
}