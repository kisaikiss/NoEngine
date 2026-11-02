#include "AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {

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
