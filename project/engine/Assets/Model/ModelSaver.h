#pragma once
#include "../AssetHandles.h"
#include "ModelAsset.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "../ModelLoader.h"

namespace NoEngine {

class ModelSaver {
public:
    ModelSaver(const ModelSaver& obj) = delete;
    ModelSaver& operator=(const ModelSaver& obj) = delete;

    static ModelSaver& Get() {
        static ModelSaver modelSaver;
        return modelSaver;
    }

    // --- Mesh ---
    Asset::MeshHandle AddMesh(Mesh&& mesh) {
        Asset::MeshHandle handle{ nextMeshId_++ };
        meshes_[handle.id] = std::move(mesh);
        return handle;
    }
    const Mesh* GetMesh(Asset::MeshHandle handle) const {
        auto it = meshes_.find(handle.id);
        return it != meshes_.end() ? &it->second : nullptr;
    }

    // --- Skeleton ---
    Asset::SkeletonHandle AddSkeleton(Skeleton&& skeleton) {
        Asset::SkeletonHandle handle{ nextSkeletonId_++ };
        skeletons_[handle.id] = std::move(skeleton);
        return handle;
    }
    const Skeleton* GetSkeleton(Asset::SkeletonHandle handle) const {
        auto it = skeletons_.find(handle.id);
        return it != skeletons_.end() ? &it->second : nullptr;
    }

    // --- Animation ---
    Asset::AnimationHandle AddAnimation(Animation&& animation) {
        Asset::AnimationHandle handle{ nextAnimationId_++ };
        animations_[handle.id] = std::move(animation);
        return handle;
    }
    const Animation* GetAnimation(Asset::AnimationHandle handle) const {
        auto it = animations_.find(handle.id);
        return it != animations_.end() ? &it->second : nullptr;
    }

    // --- Material ---
    Asset::MaterialHandle AddMaterial(Material&& material) {
        Asset::MaterialHandle handle{ nextMaterialId_++ };
        materials_[handle.id] = std::move(material);
        return handle;
    }
    const Material* GetAnimation(Asset::MaterialHandle handle) const {
        auto it = materials_.find(handle.id);
        return it != materials_.end() ? &it->second : nullptr;
    }

    void ClearAll() {
        meshes_.clear();
        skeletons_.clear();
        animations_.clear();
        materials_.clear();
    }

    ModelAsset LoadOrGetModel(const std::string& filePath) {
        // 1. 既に同じファイルがロードされているかキャッシュを確認
        auto it = modelCache_.find(filePath);
        if (it != modelCache_.end()) {
            // 既にロード済みなら、キャッシュされているHandle群をそのまま返す
            return it->second;
        }

        // 2. キャッシュにない場合は、ModelLoaderを使って新規ロード
        ModelAsset newAsset = Asset::ModelLoader::Load(filePath);

        // 3. 次回のためにキャッシュに登録
        modelCache_[filePath] = newAsset;

        return newAsset;
    }

private:
    uint32_t nextMeshId_ = 0;
    uint32_t nextSkeletonId_ = 0;
    uint32_t nextAnimationId_ = 0;
    uint32_t nextMaterialId_ = 0;

    std::unordered_map<uint32_t, Mesh> meshes_;
    std::unordered_map<uint32_t, Skeleton> skeletons_;
    std::unordered_map<uint32_t, Animation> animations_;
    std::unordered_map<uint32_t, Material> materials_;
    std::unordered_map<std::string, ModelAsset> modelCache_;
    ModelSaver() = default;
    ~ModelSaver() = default;
};
}

