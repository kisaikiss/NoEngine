#include "stdafx.h"
#include "BVH.h"

#include <numeric>

namespace NoEngine {
namespace Math {
std::shared_ptr<BVHNode> BuildBVH(const std::vector<TriangleCollider>& triangles) {
    assert(!triangles.empty());
    std::vector<int> indices(triangles.size());
    std::iota(indices.begin(), indices.end(), 0);
    return BuildBVHDetail(triangles, std::move(indices));
}

void QueryBVH(const BVHNode* node, const AABBCollider& queryBox, const std::vector<TriangleCollider>& tris, std::vector<int>& outIndices) {
    if (!node) return;
    if (!node->bounds.Overlaps(queryBox)) return;

    if (node->IsLeaf()) {
        for (int i : node->triIndices)
            outIndices.push_back(i);
        return;
    }

    QueryBVH(node->left.get(), queryBox, tris, outIndices);
    QueryBVH(node->right.get(), queryBox, tris, outIndices);
}
}


Math::AABBCollider ComputeBounds(const std::vector<Math::TriangleCollider>& tris, const std::vector<int>& indices) {
    Math::AABBCollider box{};
    for (int i : indices)
        for (const auto& v : tris[i].v)
            box.Expand(v);
    return box;
}

bool FindBestSplit(const std::vector<Math::TriangleCollider>& tris, const std::vector<int>& indices, const Math::AABBCollider& parentBounds, int& outAxis, float& outSplit) {
    constexpr int BINS = 3;        // ビン数（多いほど精度↑・速度↓）
    constexpr int LEAF_THRESHOLD = 4; // リーフにする最小三角形数

    if ((int)indices.size() <= LEAF_THRESHOLD) return false;

    float bestCost = 1e30f;
    outAxis = -1;

    // 3軸それぞれで試す
    for (int axis = 0; axis < 3; ++axis) {
        float lo = (&parentBounds.min.x)[axis];
        float hi = (&parentBounds.max.x)[axis];
        if (hi - lo < 1e-5f) continue;

        float step = (hi - lo) / BINS;

        // ビンごとに三角形数と AABB を集計
        struct Bin { Math::AABBCollider bounds; int count = 0; };
        std::array<Bin, BINS> bins;

        for (int i : indices) {
            Math::Vector3 centroid = tris[i].Centroid();
            float c = (&centroid.x)[axis];
            int b = std::clamp((int)((c - lo) / step), 0, BINS - 1);
            bins[b].bounds.Expand(tris[i].v[0]);
            bins[b].bounds.Expand(tris[i].v[1]);
            bins[b].bounds.Expand(tris[i].v[2]);
            bins[b].count++;
        }

        // 左側プレフィックス / 右側サフィックス を計算
        std::array<float, BINS - 1> costL, costR;
        {
            Math::AABBCollider   accL;
            int    cntL = 0;
            for (int i = 0; i < BINS - 1; ++i) {
                accL.Expand(bins[i].bounds);
                cntL += bins[i].count;
                costL[i] = accL.SurfaceArea() * cntL;
            }
            Math::AABBCollider   accR;
            int    cntR = 0;
            for (int i = BINS - 1; i > 0; --i) {
                accR.Expand(bins[i].bounds);
                cntR += bins[i].count;
                costR[i - 1] = accR.SurfaceArea() * cntR;
            }
        }

        for (int i = 0; i < BINS - 1; ++i) {
            float cost = costL[i] + costR[i];
            if (cost < bestCost) {
                bestCost = cost;
                outAxis = axis;
                outSplit = lo + step * (i + 1);
            }
        }
    }

    return outAxis >= 0;
}
std::shared_ptr<Math::BVHNode> BuildBVHDetail(const std::vector<Math::TriangleCollider>& tris, std::vector<int> indices, int depth) {
    constexpr int MAX_DEPTH = 20;
    constexpr int LEAF_THRESHOLD = 4;

    auto node = std::make_shared<Math::BVHNode>();
    node->bounds = ComputeBounds(tris, indices);

    bool makeLeaf = ((int)indices.size() <= LEAF_THRESHOLD) || (depth >= MAX_DEPTH);

    if (!makeLeaf) {
        int   axis = -1;
        float split = 0.f;
        makeLeaf = !FindBestSplit(tris, indices, node->bounds, axis, split);

        if (!makeLeaf) {
            std::vector<int> leftIdx, rightIdx;
            for (int i : indices) {
                Math::Vector3 centroid = tris[i].Centroid();
                float c = (&centroid.x)[axis];
                (c < split ? leftIdx : rightIdx).push_back(i);
            }

            // 片方が空の場合はリーフにフォールバック
            if (leftIdx.empty() || rightIdx.empty()) {
                makeLeaf = true;
            } else {
                node->left = BuildBVHDetail(tris, std::move(leftIdx), depth + 1);
                node->right = BuildBVHDetail(tris, std::move(rightIdx), depth + 1);
            }
        }
    }

    if (makeLeaf) {
        node->triIndices = std::move(indices);
    }

    return node;
}
}