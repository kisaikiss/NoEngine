#include "stdafx.h"
#include "ParticleTestSystem.h"

void ParticleTestSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<No::ParticleEmitterComponent>();
	int count = 0;
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		for (auto& p : emitter->particles) {
			p.transform.translate += p.velocity * deltaTime;
			p.currentTime += deltaTime;
		}

		for (uint32_t i = 0; i < emitter->particles.size(); i++) {
			if (emitter->particles[i].currentTime >= emitter->particles[i].lifeTime) {
				emitter->particles.erase(emitter->particles.begin() + i);
			}
		};

		emitter->frequencyTime += deltaTime;
		if (emitter->frequency <= emitter->frequencyTime) {
			EmitParticle(transform, emitter);
			emitter->frequencyTime -= emitter->frequency;
		}
		
		count += (int)emitter->particles.size();

	}

#ifdef USE_IMGUI
	ImGui::Begin("particleCount");
	ImGui::Text("count : %d", count);
	ImGui::End();
#endif // USE_IMGUI


}

void ParticleTestSystem::EmitParticle(No::TransformComponent* transform, No::ParticleEmitterComponent* emitter) {
	for (uint32_t count = 0; count < emitter->count; count++) {
		emitter->particles.push_back(No::Particle(No::TransformComponent(transform->translate,No::Quaternion::IDENTITY,No::Vector3::UNIT_SCALE),
			No::Vector3(No::GetRandomValNormalized(), No::GetRandomValNormalized(), No::GetRandomValNormalized())));
	}
}
