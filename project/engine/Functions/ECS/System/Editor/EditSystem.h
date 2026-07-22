#pragma once
#include "../ISystem.h"
#include "externals/nlohmann/json.hpp"
namespace NoEngine {

struct FolderTag {};
namespace ECS {
class EditSystem :
    public ISystem {
public:
    EditSystem() { SetStopInGameStop(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    struct FolderNode {
        std::string name;
        ECS::Entity folderEntity = INVALID_ENTITY; // このフォルダ自体のEntity
        std::map<std::string, FolderNode> children;
        std::vector<ECS::Entity> entities; // フォルダ以外のエンティティ
    };


    struct EditorState {
        Entity selectedEntity = INVALID_ENTITY;
    };

    EditorState editorState_;
    // ctrl + cでコピーしたオブジェクトを一時保存する変数
    nlohmann::json copyObject_;
    // コピーやペーストを毎フレーム呼ばないためのインターバル計算用変数
    float timeInterval_ = 0.0f;

    char addComponentFilter_[128] = {};

    void SaveFile(Registry& registry, nlohmann::json j);
    void LoadFile(Registry& registry);

    void DrawHierarchyWindow(Registry& registry);
    void DrawFolderNode(FolderNode& node, ECS::Registry& registry, const std::string& currentPath);
    void DrawEntityItem(ECS::Registry& registry, ECS::Entity e);
    void DrawAddComponentMenu(Registry& registry, Entity entity);
    void EnsureUniqueEditTagNames(Registry& registry);
    void CreateFolder(ECS::Registry& registry, const std::string& name, const std::string& parentPath);
    void AddEntityToFolder(FolderNode& root, ECS::Registry& registry, Entity e);
    bool FirstLoaded_ = false;
};
}
}
