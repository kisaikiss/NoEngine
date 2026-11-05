#pragma once
#include "engine/Assets/AssetHandles.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Assets/Model/Mesh.h"
#include "engine/Assets/Model/Skeleton.h"

namespace NoEngine {

namespace Component {
// 変形結果とBLASはキャラクターごとに所有する。
struct AnimatorSkinningResources {
	StructuredBuffer vertexBuffer;
	StructuredBuffer paletteResource;
	std::unique_ptr<RaytracingMesh> raytracingMesh;
};

struct AnimatorRuntimeState {
	Asset::MeshHandle meshHandle;
	Asset::SkeletonHandle skeletonHandle;
	Skeleton skeleton;
	std::vector<SkeletonWell> palette;
	// スナップショットは描画結果を保持し、次の書き込み前にGPUリソースを分離する。
	std::shared_ptr<AnimatorSkinningResources> skinning;
};

struct AnimatorComponent {
	// モデルに含まれるアニメーション名を読み込み順に取得する。
	std::vector<std::string> GetAnimationNames() const;
	// 名前が見つからない場合は再生状態を変更せず、falseを返す。
	bool SetAnimation(const std::string& name);
	AnimatorSkinningResources& EnsureSkinningResources(const Mesh& mesh);
	StructuredBuffer& GetVertexBuffer(Mesh& mesh) const;
	RaytracingMesh* GetRaytracingMesh(Mesh& mesh) const;

	std::vector<Asset::AnimationHandle> animationHandles;
	Transform local;
	uint32_t currentAnimation = 0;
	Asset::SkeletonHandle skeletonHandle;
	float time = 0.f;
	float animationSpeedMagnification = 1.0f;
	bool drawSkeleton = false;
	bool enableSkinning = false;

	// アニメーション切り替え時のブレンドにかける時間(秒)。0以下にすると即切り替えになる
	float blendDuration = 0.2f;

	// これより下はランタイム用（エディタでは基本触らない想定）
	uint32_t preCurrentAnimation = 0;   // 前フレームのcurrentAnimation。切り替え検知に使う
	uint32_t previousAnimation = 0;     // ブレンド元(切り替え前)のアニメーションIndex
	float previousAnimationTime = 0.f;  // 切り替わった瞬間に凍結した、ブレンド元の再生時刻
	bool isBlending = false;
	float blendTimer = 0.f;
	AnimatorRuntimeState runtime;
};
}
}
