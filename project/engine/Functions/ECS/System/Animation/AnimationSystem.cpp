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
	RootSignatureBuilder::BuildFromReflection(reflections, rootSignature_, ConvertString(sPsoName));
	pso_.SetRootSignature(rootSignature_);

	pso_.Finalize();
}

void AnimationSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	Update(registry, deltaTime);
	for (auto e : registry.View<Component::AnimatorComponent, Component::MeshComponent>()) {
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(e);
		auto* animeComp = registry.GetComponent<Component::AnimatorComponent>(e);

		auto& modelSaver = ModelSaver::Get();
		auto* mesh = modelSaver.GetMesh(meshComp->handle);
		if (animeComp->enableSkinning && mesh->numJoints) {
			auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));
			ctx.SetPipelineState(pso_);
			ctx.SetRootSignature(rootSignature_);
			// ジョイント
			ctx.CopyBufferRegion(mesh->paletteResource, 0, mesh->paletteUpload, 0, sizeof(SkeletonWell) * mesh->mappedPalette.size());
			ctx.TransitionResource(mesh->paletteResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
			ctx.SetDynamicDescriptor(rootIndex["gJoints"], 0, mesh->paletteResource.GetSRV());
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
			ctx.TransitionResource(mesh->useVertexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			ctx.SetDynamicDescriptor(rootIndex["gOutputVertices"], 0, mesh->useVertexBuffer.GetUAV());

			// 実行
			ctx.Dispatch1D((mesh->vertices.size() + 1023) / 1024, 1);

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
		auto& modelSaver = ModelSaver::Get();

		Animation* currentAnimation = modelSaver.GetAnimation(animeComp->animationHandles[currentIndex]);
		auto* skeleton = modelSaver.GetSkeleton(animeComp->skeletonHandle);
		if (!currentAnimation) continue;

		animeComp->time = std::fmod(animeComp->time, currentAnimation->duration);

		Animation* previousAnimation = nullptr;
		float blendT = 1.0f;
		if (animeComp->isBlending) {
			animeComp->blendTimer += deltaTime;
			previousAnimation = modelSaver.GetAnimation(animeComp->animationHandles[animeComp->previousAnimation]);
			blendT = std::clamp(animeComp->blendTimer / animeComp->blendDuration, 0.0f, 1.0f);
			if (animeComp->blendTimer >= animeComp->blendDuration) {
				animeComp->isBlending = false;
			}
		}

		if (skeleton) {
			SkeletonUpdate(animeComp, skeleton, currentAnimation, previousAnimation);
			if (animeComp->drawSkeleton) SkeletonDraw(skeleton);
			SkinUpdate(skeleton, meshComp);
		}
		auto* mesh = ModelSaver::Get().GetMesh(meshComp->handle);
		if (!mesh) return;

		if (currentAnimation->nodeAnimations.contains(mesh->rootNode.name)) {
			Transform rootLocal = SampleNodeAnimation(currentAnimation->nodeAnimations[mesh->rootNode.name], animeComp->time);

			if (previousAnimation && previousAnimation->nodeAnimations.contains(mesh->rootNode.name)) {
				Transform prevRootLocal = SampleNodeAnimation(
					previousAnimation->nodeAnimations[mesh->rootNode.name], animeComp->previousAnimationTime);
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

	for (Joint& joint : skeleton->joints) {
		// 現在のアニメーションでのローカルTransformをサンプリング(なければバインドポーズのまま)
		Transform local = joint.transform;
		if (auto it = currentAnimation->nodeAnimations.find(joint.name); it != currentAnimation->nodeAnimations.end()) {
			local = SampleNodeAnimation(it->second, animeComp->time);
		}

		// ブレンド中なら、切り替え時に凍結した旧アニメーションのポーズと補間する
		if (animeComp->isBlending && previousAnimation) {
			Transform prevLocal = joint.transform;
			if (auto it = previousAnimation->nodeAnimations.find(joint.name); it != previousAnimation->nodeAnimations.end()) {
				prevLocal = SampleNodeAnimation(it->second, animeComp->previousAnimationTime);
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

void AnimationSystem::SkinUpdate(Skeleton* skeleton, Component::MeshComponent* meshComp) {
	auto* mesh = ModelSaver::Get().GetMesh(meshComp->handle);
	if (!mesh) return;
	for (size_t jointIndex = 0; jointIndex < skeleton->joints.size(); jointIndex++) {
		mesh->mappedPalette[jointIndex].skeletonSpaceMatrix =
			skeleton->inverseBindPoseMatrices[jointIndex] * skeleton->joints[jointIndex].skeletonSpaceMatrix;
		mesh->mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			MathCalculations::Transpose(MathCalculations::Inverse(mesh->mappedPalette[jointIndex].skeletonSpaceMatrix));

		memcpy(mesh->paletteUpload.Map(), mesh->mappedPalette.data(), sizeof(SkeletonWell) * mesh->mappedPalette.size());
		
	}
}

Transform AnimationSystem::SampleNodeAnimation(const NodeAnimation& keyframes, float time) {
	Transform result;
	result.translate = CalculateValue(keyframes.translate.keyframes, time);
	result.rotation = CalculateValue(keyframes.rotation.keyframes, time);
	result.scale = CalculateValue(keyframes.scale.keyframes, time);
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