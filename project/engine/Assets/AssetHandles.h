#pragma once
namespace NoEngine {
// 無効なIDを表す定数
constexpr uint32_t INVALID_ASSET_ID = 0xFFFFFFFF;

// テンプレートを用いて型安全なハンドルを作成
template <typename T>
struct AssetHandle {
    uint32_t id = INVALID_ASSET_ID;

    bool IsValid() const { return id != INVALID_ASSET_ID; }
    bool operator==(const AssetHandle& other) const { return id == other.id; }
};

namespace Asset {
using MeshHandle = AssetHandle<struct MeshTag>;
using AnimationHandle = AssetHandle<struct AnimationTag>;
using SkeletonHandle = AssetHandle<struct SkeletonTag>;
using MaterialHandle = AssetHandle<struct MaterialTag>;
}
}