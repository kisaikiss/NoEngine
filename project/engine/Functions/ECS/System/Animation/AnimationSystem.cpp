#include "AnimationSystem.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "../../Component/Common/TransformComponent.h" 
#include "engine/Assets/Model/ModelSaver.h"


namespace NoEngine {
namespace ECS {
using namespace Math;

namespace {
std::wstring sPsoName = L"Animation PSO";
}


AnimationSystem::AnimationSystem() {
	ShaderModule animationCS(ShaderStage::Compute, L"resources/engine/Shaders/Compute/Skinning.CS.hlsl", L"cs_6_0");

	const ShaderReflection& csReflection = animationCS.GetReflection();

	
	pso_ = ComputePSO(sPsoName);

	pso_.SetComputeShader(animationCS.GetBytecode());

	std::vector<ShaderReflection> reflections;
	reflections.push_back(csReflection);
	auto rootSignature = std::make_shared<RootSignature>();
	RootSignatureBuilder::BuildFromReflection(reflections, *rootSignature, ConvertString(sPsoName));
	pso_.SetRootSignature(*rootSignature);
	rootSignature_ = std::move(rootSignature);

	pso_.Finalize();
}

void AnimationSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	Update(registry, deltaTime);
	for (auto e : registry.View<Component::AnimatorComponent, Component::MeshComponent>()) {
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(e);
		auto* animeComp = registry.GetComponent<Component::AnimatorComponent>(e);

		auto& modelSaver = ModelSaver::Get();
		auto* mesh = modelSaver.GetMesh(meshComp->handle);
		if (mesh && animeComp->enableSkinning && !mesh->skinClusterData.empty() &&
			!mesh->vertices.empty() && !animeComp->runtime.palette.empty()) {
			auto& runtime = animeComp->runtime;
			auto& skinning = animeComp->EnsureSkinningResources(*mesh);
			// StopによるRegistry復元が同じフレーム内で起きても、記録済み参照を失わない。
			ctx.KeepAlive(runtime.skinning);
			auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));
			ctx.SetPipelineState(pso_);
			ctx.SetRootSignature(*rootSignature_);
			// ジョイント
			// フレーム間でも上書きしない、フェンス管理された転送領域を使う。
			const size_t paletteSize = sizeof(SkeletonWell) * runtime.palette.size();
			auto upload = ctx.ReserveUploadMemory(paletteSize);
			memcpy(upload.DataPtr, runtime.palette.data(), paletteSize);
			ctx.CopyBufferRegion(skinning.paletteResource, 0, upload.Buffer, upload.Offset, paletteSize);
			ctx.TransitionResource(skinning.paletteResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
			ctx.SetDynamicDescriptor(rootIndex["gJoints"], 0, skinning.paletteResource.GetSRV());
			// 入力頂点 + インフルエンス
			ctx.TransitionResource(mesh->baseVertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			ctx.SetDynamicDescriptor(rootIndex["gInputVertices"], 0, mesh->baseVertexBuffer.GetSRV());
			// 頂点数
			_declspec(align(16))struct {
				uint32_t numVertices;
				uint32_t pad[3];
			}constants;
			constants.numVertices = static_cast<uint32_t>(mesh->vertices.size());
			ctx.SetDynamicConstantBufferView(rootIndex["gSkinningInformation"], sizeof(constants), &constants);
			// 出力頂点
			ctx.TransitionResource(skinning.vertexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			ctx.SetDynamicDescriptor(rootIndex["gOutputVertices"], 0, skinning.vertexBuffer.GetUAV());

			// 実行
			ctx.Dispatch1D((mesh->vertices.size() + 1023) / 1024, 1);
			// 描画キューへの受け渡しでは、Computeで扱えるCOMMONに戻す。
			ctx.TransitionResource(skinning.vertexBuffer, D3D12_RESOURCE_STATE_COMMON, true);

		}
	}

}

void AnimationSystem::Update(Registry& registry, float deltaTime) {
	AnimationUpdate(registry, deltaTime);
}

void AnimationSystem::AnimationUpdate(Registry& registry, float deltaTime) {
	auto view = registry.View<Component::AnimatorComponent, Component::MeshComponent>();

	for (auto entity : view) {
		auto* animeComp = registry.GetComponent<Component::AnimatorComponent>(entity);
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(entity);
		auto& modelSaver = ModelSaver::Get();
		auto* mesh = modelSaver.GetMesh(meshComp->handle);
		if (!mesh) continue;
		const auto* bindSkeleton = modelSaver.GetSkeleton(animeComp->skeletonHandle);
		auto& runtime = animeComp->runtime;
		if (runtime.meshHandle != meshComp->handle || runtime.skeletonHandle != animeComp->skeletonHandle) {
			runtime = {};
			runtime.meshHandle = meshComp->handle;
			runtime.skeletonHandle = animeComp->skeletonHandle;
			if (bindSkeleton) runtime.skeleton = *bindSkeleton;
			animeComp->local = mesh->rootNode.transform;
		}
		if (animeComp->currentAnimation >= animeComp->animationHandles.size()) continue;

		if (animeComp->currentAnimation != animeComp->preCurrentAnimation) {
			animeComp->previousAnimation = animeComp->preCurrentAnimation;
			animeComp->previousAnimationTime = animeComp->time;
			animeComp->isBlending = (animeComp->blendDuration > 0.0f);
			animeComp->blendTimer = 0.0f;
			animeComp->time = 0.0f;
			animeComp->preCurrentAnimation = animeComp->currentAnimation;
		}

		animeComp->time += deltaTime * animeComp->animationSpeedMagnification;
		uint32_t currentIndex = animeComp->currentAnimation;

		Animation* currentAnimation = modelSaver.GetAnimation(animeComp->animationHandles[currentIndex]);
		auto* skeleton = bindSkeleton ? &runtime.skeleton : nullptr;
		if (!currentAnimation) continue;

		animeComp->time = currentAnimation->duration > 0.0f
			? std::fmod(animeComp->time, currentAnimation->duration) : 0.0f;

		Animation* previousAnimation = nullptr;
		float blendT = 1.0f;
		if (animeComp->isBlending && animeComp->blendDuration > 0.0f &&
			animeComp->previousAnimation < animeComp->animationHandles.size()) {
			animeComp->blendTimer += deltaTime;
			previousAnimation = modelSaver.GetAnimation(animeComp->animationHandles[animeComp->previousAnimation]);
			blendT = std::clamp(animeComp->blendTimer / animeComp->blendDuration, 0.0f, 1.0f);
			if (animeComp->blendTimer >= animeComp->blendDuration) {
				animeComp->isBlending = false;
			}
		} else {
			animeComp->isBlending = false;
		}

		if (skeleton) {
			SkeletonUpdate(animeComp, skeleton, currentAnimation, previousAnimation);
			if (animeComp->drawSkeleton) SkeletonDraw(skeleton);
			SkinUpdate(skeleton, animeComp);
		}
		animeComp->local = mesh->rootNode.transform;

		if (currentAnimation->nodeAnimations.contains(mesh->rootNode.name)) {
			Transform rootLocal = SampleNodeAnimation(currentAnimation->nodeAnimations[mesh->rootNode.name], animeComp->time, mesh->rootNode.transform);

			if (previousAnimation && previousAnimation->nodeAnimations.contains(mesh->rootNode.name)) {
				Transform prevRootLocal = SampleNodeAnimation(
					previousAnimation->nodeAnimations[mesh->rootNode.name], animeComp->previousAnimationTime, mesh->rootNode.transform);
				rootLocal.translate = Easing::Lerp(prevRootLocal.translate, rootLocal.translate, blendT);
				rootLocal.rotation = Math::Quaternion::Slerp(prevRootLocal.rotation, rootLocal.rotation, blendT);
				rootLocal.scale = Easing::Lerp(prevRootLocal.scale, rootLocal.scale, blendT);
			}

			animeComp->local = rootLocal;
		}
	}
}
void AnimationSystem::SkeletonUpdate(Component::AnimatorComponent* animeComp, Skeleton* skeleton, Animation* currentAnimation, Animation* previousAnimation) {
	float blendT = 1.0f;
	if (animeComp->isBlending && animeComp->blendDuration > 0.0f) {
		blendT = std::clamp(animeComp->blendTimer / animeComp->blendDuration, 0.0f, 1.0f);
	}

	const auto* bindSkeleton = ModelSaver::Get().GetSkeleton(animeComp->skeletonHandle);
	for (Joint& joint : skeleton->joints) {
		const Transform& bindPose = bindSkeleton->joints[joint.index].transform;
		// 現在のアニメーションでのローカルTransformをサンプリング(なければバインドポーズのまま)
		Transform local = bindPose;
		if (auto it = currentAnimation->nodeAnimations.find(joint.name); it != currentAnimation->nodeAnimations.end()) {
			local = SampleNodeAnimation(it->second, animeComp->time, bindPose);
		}

		// ブレンド中なら、切り替え時に凍結した旧アニメーションのポーズと補間する
		if (animeComp->isBlending && previousAnimation) {
			Transform prevLocal = bindPose;
			if (auto it = previousAnimation->nodeAnimations.find(joint.name); it != previousAnimation->nodeAnimations.end()) {
				prevLocal = SampleNodeAnimation(it->second, animeComp->previousAnimationTime, bindPose);
			}
			local.translate = Easing::Lerp(prevLocal.translate, local.translate, blendT);
			local.rotation = Math::Quaternion::Slerp(prevLocal.rotation, local.rotation, blendT);
			local.scale = Easing::Lerp(prevLocal.scale, local.scale, blendT);
		}

		joint.transform = local;

		joint.localMatrix = joint.transform.MakeAffineMatrix4x4();
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton->joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void AnimationSystem::SkeletonDraw(Skeleton* skeleton) {

	std::vector<Joint>& joints = skeleton->joints;
	for (Joint& joint : joints) {
		if (joint.parent.has_value()) {
			const Joint& parentJoint = joints[joint.parent.value()];

			Vector3 start = parentJoint.skeletonSpaceMatrix.GetTranslate();
			Vector3 end = joint.skeletonSpaceMatrix.GetTranslate();

			DebugPrimitive::DrawLine(start, end, Color::WHITE);
		}
	}
}

void AnimationSystem::SkinUpdate(const Skeleton* skeleton, Component::AnimatorComponent* animeComp) {
	auto& palette = animeComp->runtime.palette;
	palette.resize(skeleton->joints.size());
	for (size_t jointIndex = 0; jointIndex < skeleton->joints.size(); jointIndex++) {
		palette[jointIndex].skeletonSpaceMatrix =
			skeleton->inverseBindPoseMatrices[jointIndex] * skeleton->joints[jointIndex].skeletonSpaceMatrix;
		palette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			MathCalculations::Transpose(MathCalculations::Inverse(palette[jointIndex].skeletonSpaceMatrix));
		
	}
}

Transform AnimationSystem::SampleNodeAnimation(const NodeAnimation& keyframes, float time, const Transform& bindPose) {
	Transform result = bindPose;
	if (!keyframes.translate.keyframes.empty()) result.translate = CalculateValue(keyframes.translate.keyframes, time);
	if (!keyframes.rotation.keyframes.empty()) result.rotation = CalculateValue(keyframes.rotation.keyframes, time);
	if (!keyframes.scale.keyframes.empty()) result.scale = CalculateValue(keyframes.scale.keyframes, time);
	return result;
}

Vector3 AnimationSystem::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	if (keyframes.empty()) {
		return Vector3();
	}
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnext Indexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}


Quaternion AnimationSystem::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	if (keyframes.empty()) {
		return Quaternion();
	}
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnext Indexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return MathCalculations::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}


}
}
#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::AnimationSystem, "AnimationSystem", "Animation")
