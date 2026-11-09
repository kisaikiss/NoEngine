#pragma once
#include "../AssetHandles.h"
namespace NoEngine {

struct ModelAsset {
    Asset::MeshHandle mesh;
    Asset::SkeletonHandle skeleton;
    std::vector<Asset::AnimationHandle> animations;
};

}
