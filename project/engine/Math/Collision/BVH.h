#pragma once
#include "ColliderComponent.h"

namespace NoEngine {
namespace Math {
struct BVHNode {
    AABBCollider bounds;

    // 内部ノード: left/right が非 null
    // リーフノード: left/right が null、triIndices に三角形インデックス
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<int>         triIndices; // リーフのみ使用

    bool IsLeaf() const { return !left && !right; }
};
}
}
