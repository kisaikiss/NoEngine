#include "EffectPreviewPanel.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/VelocityComponent.h"
#include "engine/Functions/ECS/System/PlayModeController.h"
#include "engine/Functions/Renderer/RenderPass/Rasterization/CPUParticlePass.h"
#include "engine/Functions/Scene/SceneNameComponent.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine::Editor {
using namespace Component;

void EffectPreviewPanel::Draw(ECS::Registry& source, ECS::Entity entity) {
#ifdef USE_IMGUI
	if (!source.Has<EffectEmitterComponent>(entity)) return;
	if (source_ != &source || entity_ != entity) {
		Reset();
		source_ = &source;
		entity_ = entity;
	}
	selectionSeen_ = true;

	ImGui::PushID("EffectPreview");
	ImGui::SeparatorText("エフェクト試し撃ち");
	const bool inPlayMode = ECS::PlayModeController::IsInPlayMode();
	ImGui::BeginDisabled(inPlayMode);
	ImGui::InputScalar("固定シード", ImGuiDataType_U32, &seed_);
	if (ImGui::Button("再生")) Play(source, entity);
	ImGui::SameLine();
	if (ImGui::Button("停止")) emitter_ = {};
	ImGui::SameLine();
	if (ImGui::Button("リセット")) Reset();
	ImGui::EndDisabled();
	ImGui::Text("生存中: %zu / %zu", alive_, kParticleLimit);
	ImGui::Text("上限で発生できなかった数: %llu", static_cast<unsigned long long>(dropped_));
	ImGui::TextDisabled("%s", inPlayMode ? "ゲームを停止して試し撃ちできます" : "Scene ビューに表示");
	ImGui::PopID();
#else
	static_cast<void>(source);
	static_cast<void>(entity);
#endif
}

void EffectPreviewPanel::Play(ECS::Registry& source, ECS::Entity entity) {
	Reset();
	const auto* preset = source.GetComponent<EffectEmitterComponent>(entity);
	if (!preset) return;

	const auto* transform = source.GetComponent<TransformComponent>(entity);
	const auto position = transform ? transform->GetWorldPosition(source) : Math::Vector3::ZERO;
	const auto rotation = transform ? transform->rotation : Math::Quaternion::IDENTITY;
	particles_ = std::make_shared<ECS::Registry>();
	random_.seed(seed_);
	particles_->PlayEffect(*preset, position, rotation);
	Step(0.0f);
}

void EffectPreviewPanel::Reset() {
	emitter_ = {};
	particles_.reset();
	accumulator_ = 0.0;
	alive_ = 0;
	dropped_ = 0;
}

void EffectPreviewPanel::Update(ECS::Registry& scene, float deltaTime) {
	if (!selectionSeen_ || ECS::PlayModeController::IsInPlayMode()) {
		Reset();
		source_ = nullptr;
		entity_ = ECS::INVALID_ENTITY;
	}
	selectionSeen_ = false;
	if (!particles_) return;

	// 描画フレームの間隔で発生順序や寿命の判定が変わらないようにする。
	constexpr double step = 1.0 / 60.0;
	if (std::isfinite(deltaTime) && deltaTime > 0.0f) {
		accumulator_ += std::min<double>(deltaTime, 0.25);
		while (accumulator_ >= step) {
			Step(static_cast<float>(step));
			accumulator_ -= step;
		}
	}
	auto roots = scene.View<SceneNameComponent>();
	const auto root = roots.begin();
	if (root != roots.end()) {
		scene.AddComponent<Render::EffectPreviewSource>(*root)->particles = particles_;
	}
}

void EffectPreviewPanel::Step(float deltaTime) {
	for (auto entity : particles_->View<ParticleComponent, TransformComponent, VelocityComponent>()) {
		auto* particle = particles_->GetComponent<ParticleComponent>(entity);
		particle->currentTime += deltaTime;
		if (particle->currentTime >= particle->lifeTime) {
			particles_->DestroyEntity(entity);
			--alive_;
			continue;
		}
		auto* transform = particles_->GetComponent<TransformComponent>(entity);
		transform->translate += particles_->GetComponent<VelocityComponent>(entity)->linear * deltaTime;
	}
	particles_->FlushDestroy();

	const auto stats = emitter_.UpdatePreview(*particles_, deltaTime, kParticleLimit - alive_, random_);
	alive_ += stats.spawned;
	dropped_ += stats.dropped;
}

}
