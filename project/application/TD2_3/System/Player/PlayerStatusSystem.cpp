#include "PlayerStatusSystem.h"
#include "../../Component/PlayerstatusComponent.h"
#include "../../tag.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/BallTrailComponent.h"
#include "../../Component/ColliderComponent.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "externals/imgui/imgui.h"

void PlayerStatusSystem::Update(No::Registry& registry, float deltaTime)
{
	(void)deltaTime;
	auto playerStatusView = registry.View<PlayerStatusComponent>();

	for (auto entity : playerStatusView)
	{
		auto* status = registry.GetComponent<PlayerStatusComponent>(entity);
		
		// 経験値が閾値を超えたらレベルアップを開始（UI 表示のため pendingUpgrade を使う）
		const int requiredExp = 3;
		if (!status->pendingUpgrade && status->exp >= requiredExp)
		{
			status->exp = 0;
			status->level++;
			// UI 表示開始フラグ
			status->pendingUpgrade = true;
		}

		// レベルアップの選択 UI を表示中なら ImGui で三択を表示
#ifdef USE_IMGUI
		ImGui::Begin("Debug Player Status");
		ImGui::Text("Score: %d", status->score);
		ImGui::Text("Level: %d", status->level);
		ImGui::Text("EXP: %d ", status->exp);
		ImGui::Text("HP: %d / %d", status->hp, status->hpMax);
		ImGui::End();

		if (status->pendingUpgrade)
		{
			ImGui::SetNextWindowSize(ImVec2(400, 160), ImGuiCond_Once);
			ImGui::Begin("Level Up!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

			ImGui::Text("レベルアップ！ 強化を1つ選んでください。");
			ImGui::Spacing();

			// Option 1 : Vaus の幅を少し増やす
			if (ImGui::Button("パドルを広げる", ImVec2(360, 0)))
			{
				// 全ての Vaus に対して幅倍率を適用
				for (auto vausEntity : registry.View<VausTag, VausStateComponent>())
				{
					auto* vausState = registry.GetComponent<VausStateComponent>(vausEntity);
					// 過度に増えないよう比例で増加
					vausState->widthScale += 0.05f;
				}

				status->pendingUpgrade = false;
			}
			ImGui::Spacing();

			// Option 2 : HP 上限を +1
			if (ImGui::Button("HP上限 +1", ImVec2(360, 0)))
			{
				status->hpMax += 1;
				// 回復もする（上限に合わせる）
				status->hp = std::min(status->hp + 1, status->hpMax);
				status->pendingUpgrade = false;
			}
			ImGui::Spacing();

			// Option 3 : 小球を 1 つ追加
			if (ImGui::Button("ボールを +1 個増やす", ImVec2(360, 0)))
			{
				// まず Vaus の位置を取得（あれば）
				NoEngine::Vector3 spawnPos = NoEngine::Vector3::ZERO;
				bool foundLastBall = false;
				for (auto ve : registry.View<BallTag, No::TransformComponent>())
				{
					auto* tr = registry.GetComponent<No::TransformComponent>(ve);
					spawnPos = tr->translate;
					// 少し内側に置く
					// spawn を一つ決めればよい
					foundLastBall = true;
				}
				if (foundLastBall)
				{
					// spawnPos をリング中心方向に押し出してボール配置
					NoEngine::Vector3 dir = NoEngine::MathCalculations::Normalize(spawnPos);
					NoEngine::Vector3 ballPos = spawnPos + dir * 0.5f;

					// 新しいエンティティを作成し、InitBall と同等のコンポーネントを追加
					auto ballEntity = registry.GenerateEntity();
					registry.AddComponent<BallTag>(ballEntity);
					registry.AddComponent<PhysicsComponent>(ballEntity);
					auto* ballState = registry.AddComponent<BallStateComponent>(ballEntity);
					ballState->landed = false;

					auto* trailComp = registry.AddComponent<BallTrailComponent>(ballEntity);
					trailComp->maxAge = 0.6f;
					trailComp->sampleInterval = 0.02f;
					trailComp->thickness = 0.35f;
					trailComp->maxSamples = 256;

					auto* collider = registry.AddComponent<SphereColliderComponent>(ballEntity);
					collider->radius = 0.25f;
					collider->colliderType = ColliderMask::kBall;
					collider->collideMask = ColliderMask::kEnemy;

					registry.AddComponent<DeathFlag>(ballEntity);

					auto* transform = registry.AddComponent<No::TransformComponent>(ballEntity);
					transform->translate = ballPos;

					auto* model = registry.AddComponent<No::MeshComponent>(ballEntity);
					NoEngine::ModelLoader::LoadModel("ball", "resources/game/td_2304/Model/ball/ball.obj", model);

					auto m = registry.AddComponent<No::MaterialComponent>(ballEntity);
					m->materials = NoEngine::ModelLoader::GetMaterial("ball");

					m->psoName = L"Renderer : Default PSO";
					m->psoId = NoEngine::Render::GetPSOID(m->psoName);
					m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
				}

				status->pendingUpgrade = false;
			}

			ImGui::End();
		}
#endif // USE_IMGUI
	}
}