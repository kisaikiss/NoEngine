#pragma once
#include "engine/Assets/AssetHandles.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Assets/Model/Mesh.h"
#include "engine/Assets/Model/Skeleton.h"

namespace NoEngine {
namespace ECS { class AnimationSystem; }
struct Animation;

namespace Component {
enum class AnimationPlaybackState { Playing, Paused, Stopped, Finished };

struct AnimationEventMarker {
	std::string animationName;
	float time = 0.0f;
	std::string name;
	std::string parameter;
};

struct TriggeredAnimationEvent {
	std::string animationName;
	float time = 0.0f;
	std::string name;
	std::string parameter;
	bool reverse = false;
};

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
	// 明示的なPlayは同じクリップでも先頭から再生する。falseで単発再生する。
	bool Play(const std::string& name, bool shouldLoop = true);
	bool Play();
	void Pause();
	bool Resume();
	// 先頭のポーズに戻して停止する。表示の更新は次回のAnimationSystemで行う。
	void Stop();
	bool Restart();
	// 秒単位で範囲内に移動し、ブレンドと終了状態を解除する。再生・一時停止は維持する。
	bool Seek(float seconds);
	bool IsPlaying() const;
	bool IsPaused() const { return playbackState == AnimationPlaybackState::Paused; }
	bool IsFinished() const { return playbackState == AnimationPlaybackState::Finished; }
	float GetDuration() const;
	float GetNormalizedTime() const;
	// 指定したクリップの範囲内にマーカーを追加する。不正な指定では変更しない。
	bool AddEventMarker(const std::string& animationName, float seconds,
		const std::string& name, const std::string& parameter = {});
	// 発火一覧は次回のAnimationSystem更新で消える。ConsumeEventsは取得時に消費する。
	const std::vector<TriggeredAnimationEvent>& GetTriggeredEvents() const { return triggeredEvents; }
	std::vector<TriggeredAnimationEvent> ConsumeEvents();
	uint64_t GetDroppedEventCount() const { return droppedEventCount; }
	static constexpr size_t kMaxEventsPerUpdate = 1024;
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
	bool loop = true;
	std::vector<AnimationEventMarker> animationEvents;

	// アニメーション切り替え時のブレンドにかける時間(秒)。0以下にすると即切り替えになる
	float blendDuration = 0.2f;

	// これより下はランタイム用（エディタでは基本触らない想定）
	uint32_t preCurrentAnimation = 0;   // 前フレームのcurrentAnimation。切り替え検知に使う
	uint32_t previousAnimation = 0;     // ブレンド元(切り替え前)のアニメーションIndex
	float previousAnimationTime = 0.f;  // 切り替わった瞬間に凍結した、ブレンド元の再生時刻
	bool isBlending = false;
	float blendTimer = 0.f;
	AnimationPlaybackState playbackState = AnimationPlaybackState::Playing;
	AnimatorRuntimeState runtime;
	std::vector<TriggeredAnimationEvent> triggeredEvents;
	uint64_t droppedEventCount = 0;
	bool eventStartPending = true;

private:
	friend class NoEngine::ECS::AnimationSystem;
	const Animation* GetCurrentAnimation() const;
	void OnAnimationChanged();
	void UpdatePlayback(float deltaTime);
	void CollectAnimationEvents(double step, bool reverse);
};
}
}
