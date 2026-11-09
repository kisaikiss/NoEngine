#pragma once
#include "engine/Functions/ECS/System/Effect/EffectEmitSystem.h"

namespace NoEngine::Editor {

class EffectPreviewPanel {
public:
	void Draw(ECS::Registry& source, ECS::Entity entity);
	void Update(ECS::Registry& scene, float deltaTime);
	void Reset();

private:
	static constexpr size_t kParticleLimit = 4096;

	void Play(ECS::Registry& source, ECS::Entity entity);
	void Step(float deltaTime);

	ECS::Registry* source_ = nullptr;
	ECS::Entity entity_ = ECS::INVALID_ENTITY;
	bool selectionSeen_ = false;
	uint32_t seed_ = 1;
	std::mt19937 random_;
	ECS::EffectEmitSystem emitter_;
	std::shared_ptr<ECS::Registry> particles_;
	double accumulator_ = 0.0;
	size_t alive_ = 0;
	uint64_t dropped_ = 0;
};

}
