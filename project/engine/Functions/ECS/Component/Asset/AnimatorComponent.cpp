#include "AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {

AnimatorSkinningResources& AnimatorComponent::EnsureSkinningResources(const Mesh& mesh) {
	if (!runtime.skinning || runtime.skinning.use_count() > 1) {
		auto skinning = std::make_shared<AnimatorSkinningResources>();
		// 全頂点をComputeで上書きする。初期転送によるGENERIC_READへの遷移を避ける。
		skinning->vertexBuffer.Create(L"Animator vertices",
			static_cast<uint32_t>(mesh.vertices.size()), sizeof(Vertex));
		skinning->paletteResource.Create(L"Animator joints",
			static_cast<uint32_t>(runtime.palette.size()), sizeof(SkeletonWell));
		runtime.skinning = std::move(skinning);
	}
	return *runtime.skinning;
}

StructuredBuffer& AnimatorComponent::GetVertexBuffer(Mesh& mesh) const {
	return enableSkinning && runtime.skinning ? runtime.skinning->vertexBuffer : mesh.useVertexBuffer;
}

RaytracingMesh* AnimatorComponent::GetRaytracingMesh(Mesh& mesh) const {
	return enableSkinning && runtime.skinning ? runtime.skinning->raytracingMesh.get() : mesh.raytracingMesh.get();
}

std::vector<std::string> AnimatorComponent::GetAnimationNames() const {
	std::vector<std::string> names;
	names.reserve(animationHandles.size());
	for (const auto handle : animationHandles) {
		if (const auto* animation = ModelSaver::Get().GetAnimation(handle)) {
			names.push_back(animation->name);
		}
	}
	return names;
}

bool AnimatorComponent::SetAnimation(const std::string& name) {
	if (name.empty()) return false;
	for (size_t index = 0; index < animationHandles.size(); ++index) {
		const auto* animation = ModelSaver::Get().GetAnimation(animationHandles[index]);
		if (animation && animation->name == name) {
			currentAnimation = static_cast<uint32_t>(index);
			return true;
		}
	}
	return false;
}

}

REFLECT_STRUCT_BEGIN(NoEngine::Component::AnimatorComponent, "Asset")
REFLECT_FIELD(currentAnimation),
REFLECT_FIELD(animationSpeedMagnification),
REFLECT_FIELD(drawSkeleton),
REFLECT_FIELD(enableSkinning),
REFLECT_FIELD(blendDuration)
REFLECT_STRUCT_END(NoEngine::Component::AnimatorComponent)
