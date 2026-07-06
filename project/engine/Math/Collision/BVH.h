#pragma once
#include "ColliderComponent.h"
#include <DirectXMath.h>

namespace NoEngine {
namespace Math {
struct BVHNode {
    AABBCollider bounds;

    // 内部ノード: left/right が非 null
    // リーフノード: left/right が null、triIndices に三角形インデックス
    std::shared_ptr<BVHNode> left;
    std::shared_ptr<BVHNode> right;
    std::vector<int>         triIndices; // リーフのみ使用

    bool IsLeaf() const { return !left && !right; }
};

/// <summary>
/// 三角形リストから BVH ツリーを構築する。
/// 地形ロード時に一度だけ呼ぶ。
/// </summary>
std::shared_ptr<BVHNode> BuildBVH(const std::vector<TriangleCollider>& triangles);

/// <summary>
/// AABB と交差する三角形インデックスをすべて収集する（Broad phase 用）。
/// </summary>
void QueryBVH(
    const BVHNode* node,
    const AABBCollider& queryBox,
    const std::vector<TriangleCollider>& tris,
    std::vector<int>& outIndices);
}

// 三角形リストから AABB を計算
Math::AABBCollider ComputeBounds(
    const std::vector<Math::TriangleCollider>& tris,
    const std::vector<int>& indices);

// SAH（Surface Area Heuristic）による最適分割軸・位置を探索
// 候補ビン数を絞って O(N log N) ではなく O(N * BINS) に抑える
bool FindBestSplit(
    const std::vector<Math::TriangleCollider>& tris,
    const std::vector<int>& indices,
    const Math::AABBCollider& parentBounds,
    int& outAxis,
    float& outSplit);

// 再帰的にノードを構築
std::shared_ptr<Math::BVHNode> BuildBVHDetail(
    const std::vector<Math::TriangleCollider>& tris,
    std::vector<int>             indices,
    int                          depth = 0);

}
