#include "UpgradeSelectionSystem.h"
#include "../../Component/PlayerstatusComponent.h"
#include "../../Component/UpgradeChooseComponent.h"
#include "../../tag.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Input/Input.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Math/Easing.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/BallTrailComponent.h"
#include "../../Component/ColliderComponent.h"
#include "engine/Assets/ModelLoader.h"
#include <random>
#include <Windows.h>

using namespace NoEngine;

namespace
{
	constexpr float kSpacing = 380.f;            // 選択肢間隔（px）
	constexpr float kSelectedYOffset = -30.f;    // 選択時に上へ移動する量
	constexpr float kAnimSpeed = 10.f;           // 補間速度
	constexpr float kInputCooldown = 0.12f;      // 入力間隔
	constexpr float kConfirmLockTime = 0.25f;	 //UI 表示直後に A ボタンを無効にする時間
	float inputCounter = 0.f;
}

// ヘルパー：クライアント座標を GraphicsCore の描画解像度に合わせてスケーリングしたマウス位置を返す
static void GetScaledMousePos(float& outX, float& outY)
{
	POINT pt{};
	outX = outY = 0.f;
	if (!GetCursorPos(&pt)) return;

	HWND hWnd = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowHandle();
	// まずクライアント座標へ
	ScreenToClient(hWnd, &pt);

	// 実クライアント矩形（論理）と、GraphicsCore が使う描画サイズ（ピクセル）を比較してスケールする
	RECT rc{};
	GetClientRect(hWnd, &rc);
	float clientW = static_cast<float>(rc.right - rc.left);
	float clientH = static_cast<float>(rc.bottom - rc.top);

	auto size = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
	float viewW = static_cast<float>(size.clientWidth);
	float viewH = static_cast<float>(size.clientHeight);

	float sx = (clientW > 0.f) ? (viewW / clientW) : 1.f;
	float sy = (clientH > 0.f) ? (viewH / clientH) : 1.f;

	outX = static_cast<float>(pt.x) * sx;
	outY = static_cast<float>(pt.y) * sy;
}

void UpgradeSelectionSystem::Update(No::Registry& registry, float deltaTime)
{
	// PlayerStatus の存在チェック（単一想定）
	auto statusView = registry.View<PlayerStatusComponent>();
	if (statusView.Empty()) return;

	// UpgradeChooseComponent を持つエンティティがあれば使う（単一想定）
	auto chooseView = registry.View<UpgradeChooseComponent>();
	UpgradeChooseComponent* choose = nullptr;
	No::Entity chooseEntity = No::Entity();
	if (chooseView.Empty())
	{
		// 自動作成（最初の Update で StatusSpriteTag のスプライトを収集してコンポーネントをアタッチ）
		// Create a manager entity
		chooseEntity = registry.GenerateEntity();
		choose = registry.AddComponent<UpgradeChooseComponent>(chooseEntity);

		// 検出: StatusSpriteTag の付いたスプライトを収集
		int optIdx = 0;
		int frameIdx = 0;
		for (auto e : registry.View<StatusSpriteTag, No::SpriteComponent, No::Transform2DComponent>())
		{
			auto* sp = registry.GetComponent<No::SpriteComponent>(e);
			if (!sp) continue;
			if (sp->name == "buttonFrame")
			{
				if (frameIdx < 3)
				{
					choose->frameEntities[frameIdx] = e;
					// frame は下になるように layer を小さく
					sp->layer = 0;
					sp->isVisible = false;
					frameIdx++;
				}
			}
			else
			{
				// option 登録（3つ想定）
				if (optIdx < 3)
				{
					choose->optionEntities[optIdx] = e;
					choose->optionNames[optIdx] = sp->name;
					// option は上レイヤー
					sp->layer = 1;
					sp->isVisible = false;
					optIdx++;
				}
			}
		}
	}
	else
	{
		for (auto e : chooseView)
		{
			choose = registry.GetComponent<UpgradeChooseComponent>(e);
			chooseEntity = e;
		}
	}

	PlayerStatusComponent* status = nullptr;
	No::Entity statusEnt = No::Entity();
	for (auto e : statusView)
	{
		status = registry.GetComponent<PlayerStatusComponent>(e);
		statusEnt = e;
	}
	if (!status) return;

	// 初期化：pendingUpgrade が立ったら active にする
	if (!choose->active && status->pendingUpgrade)
	{
		std::random_device rd;
		std::mt19937 g(rd());

		std::vector<int> idxs = { 0,1,2 };
		std::shuffle(idxs.begin(), idxs.end(), g);

		std::array<No::Entity, 3> oldEnt = choose->optionEntities;
		std::array<std::string, 3> oldNames = choose->optionNames;
		for (int i = 0; i < 3; ++i)
		{
			choose->optionEntities[i] = oldEnt[idxs[i]];
			choose->optionNames[i] = oldNames[idxs[i]];
		}

		auto* main = GraphicsCore::gWindowManager.GetMainWindow();
		auto size = main->GetWindowSize();
		float cx = size.clientWidth * 0.5f;
		float cy = size.clientHeight * 0.5f;
		choose->baseY = cy;

		choose->baseX[0] = cx - kSpacing;
		choose->baseX[1] = cx;
		choose->baseX[2] = cx + kSpacing;

		for (int i = 0; i < 3; ++i)
		{
			auto ent = choose->optionEntities[i];
			if (registry.Has<No::Transform2DComponent>(ent))
			{
				auto* tr = registry.GetComponent<No::Transform2DComponent>(ent);
				tr->translate.x = choose->baseX[i];
				tr->translate.y = choose->baseY;
			}
			if (registry.Has<No::SpriteComponent>(ent))
			{
				auto* sp = registry.GetComponent<No::SpriteComponent>(ent);
				sp->isVisible = true;
				sp->layer = 1;
			}
			choose->animOffset[i] = 0.f;
		}

		// frame: 各オプションごとに1個ずつ配置（frame も初期 Y を合わせる）
		for (int i = 0; i < 3; ++i)
		{
			auto frameEnt = choose->frameEntities[i];
			if (frameEnt != No::Entity() && registry.Has<No::Transform2DComponent>(frameEnt))
			{
				auto* trf = registry.GetComponent<No::Transform2DComponent>(frameEnt);
				trf->translate.x = choose->baseX[i];
				trf->translate.y = choose->baseY;
				if (registry.Has<No::SpriteComponent>(frameEnt))
				{
					auto* spf = registry.GetComponent<No::SpriteComponent>(frameEnt);
					spf->isVisible = true;
					spf->layer = 0;
				}
			}
		}

		choose->active = true;
		choose->selectedIndex = 1;
		choose->inputCooldown = 0.f;
		choose->confirmLock = kConfirmLockTime;
		choose->wasMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

		No::SoundPlay("levelUp", 1.0f, false);
	}

	// UI 表示中の入力・アニメーション処理
	if (choose->active)
	{
		choose->inputCooldown = std::max(0.f, choose->inputCooldown - deltaTime);
		choose->confirmLock = std::max(0.f, choose->confirmLock - deltaTime);
		// パッド左右で選択移動 (トリガー検出)
		if (choose->inputCooldown <= 0.f)
		{
			const auto& stick = Input::Pad::GetStick();
			if (Input::Pad::IsTrigger(Input::GamepadButton::Left) || stick.leftStickX < -0.2f)
			{
				choose->selectedIndex = (choose->selectedIndex + 2) % 3; // left
				choose->inputCooldown = kInputCooldown;
			}
			else if (Input::Pad::IsTrigger(Input::GamepadButton::Right) || stick.leftStickX > 0.2f)
			{
				choose->selectedIndex = (choose->selectedIndex + 1) % 3; // right
				choose->inputCooldown = kInputCooldown;
			}
		}

		// マウス位置取得（描画サイズへスケーリング）
		float mx = 0.f, my = 0.f;
		GetScaledMousePos(mx, my);

		// マウスクリック検出（押下→離した瞬間で判定）
		bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		bool mouseClicked = false;
		if (choose->wasMouseDown && !mouseDown)
		{
			mouseClicked = true;
		}
		choose->wasMouseDown = mouseDown;

		// マウスでホバーしていたら選択を変更（pointer inside sprite rect）
		for (int i = 0; i < 3; ++i)
		{
			auto ent = choose->optionEntities[i];
			if (!registry.Has<No::Transform2DComponent>(ent) || !registry.Has<No::SpriteComponent>(ent)) continue;
			auto* tr = registry.GetComponent<No::Transform2DComponent>(ent);
			auto* sp = registry.GetComponent<No::SpriteComponent>(ent);

			// sprite のサイズは transform->scale
			float sx = tr->scale.x;
			float sy = tr->scale.y;

			// pivot を考慮した左上座標
			float left = tr->translate.x - sx * sp->pivot.x;
			float top = tr->translate.y - sy * sp->pivot.y;
			float right = left + sx;
			float bottom = top + sy;

			// マウス座標が矩形内なら hover
			if (mx >= left && mx <= right && my >= top && my <= bottom)
			{
				choose->selectedIndex = i;
				// クリックで決定
				if (mouseClicked)
				{
					ApplyUpgradeChoice(registry, statusEnt, choose->optionNames[i], choose);
					HideChooseUI(registry, choose, chooseEntity);

					status->pendingUpgrade = false;
					status->exp = 0;
					status->requiredExp += 2;
					choose->active = false;
					return;
				}
			}
		}

		// パッド A で確定
		if (choose->confirmLock <= 0.f && Input::Pad::IsTrigger(Input::GamepadButton::A))
		{
			int i = choose->selectedIndex;
			ApplyUpgradeChoice(registry, statusEnt, choose->optionNames[i], choose);
			HideChooseUI(registry, choose, chooseEntity);

			status->pendingUpgrade = false;
			status->exp = 0;
			status->requiredExp += 2;
			choose->active = false;
			return;
		}

		// アニメーション補間: 各 option の targetOffset を決める
		for (int i = 0; i < 3; ++i)
		{
			float target = (i == choose->selectedIndex) ? kSelectedYOffset : 0.f;
			// animOffset は現在値、補間して translate.y に反映
			choose->animOffset[i] = NoEngine::Easing::Lerp(choose->animOffset[i], target, std::min(1.f, deltaTime * kAnimSpeed));
			// 適用 (option)
			auto ent = choose->optionEntities[i];
			if (registry.Has<No::Transform2DComponent>(ent))
			{
				auto* tr = registry.GetComponent<No::Transform2DComponent>(ent);
				tr->translate.x = choose->baseX[i];
				tr->translate.y = choose->baseY + choose->animOffset[i];
			}
			// 同じオフセットを frame にも適用（frame が option の下に追従して上へ動く）
			auto frameEnt = choose->frameEntities[i];
			if (frameEnt != No::Entity() && registry.Has<No::Transform2DComponent>(frameEnt))
			{
				auto* trf = registry.GetComponent<No::Transform2DComponent>(frameEnt);
				trf->translate.x = choose->baseX[i];
				trf->translate.y = choose->baseY + choose->animOffset[i];
			}
		}
	}
	else
	{
		// 非表示時は念のためスプライトを不可視にする（status->pendingUpgrade を false にした際の保険）
		for (int i = 0; i < 3; ++i)
		{
			auto ent = choose->optionEntities[i];
			if (registry.Has<No::SpriteComponent>(ent))
			{
				auto* sp = registry.GetComponent<No::SpriteComponent>(ent);
				sp->isVisible = false;
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			auto frameEnt = choose->frameEntities[i];
			if (frameEnt != No::Entity() && registry.Has<No::SpriteComponent>(frameEnt))
			{
				auto* spf = registry.GetComponent<No::SpriteComponent>(frameEnt);
				spf->isVisible = false;
			}
		}
	}
}

//UI を完全に非表示にする
void UpgradeSelectionSystem::HideChooseUI(No::Registry& registry, UpgradeChooseComponent* choose, No::Entity chooseEntity)
{
	(void)chooseEntity;
	for (int i = 0; i < 3; ++i)
	{
		auto ent = choose->optionEntities[i];
		if (registry.Has<No::SpriteComponent>(ent))
		{
			auto* sp = registry.GetComponent<No::SpriteComponent>(ent);
			sp->isVisible = false;
		}
	}
	for (int i = 0; i < 3; ++i)
	{
		auto frameEnt = choose->frameEntities[i];
		if (frameEnt != No::Entity() && registry.Has<No::SpriteComponent>(frameEnt))
		{
			auto* spf = registry.GetComponent<No::SpriteComponent>(frameEnt);
			spf->isVisible = false;
		}
	}

}

//選択確定時の効果を適用
void UpgradeSelectionSystem::ApplyUpgradeChoice(No::Registry& registry, No::Entity statusEntity, const std::string& choiceName, UpgradeChooseComponent* choose)
{
	(void)choose;
	auto* status = registry.GetComponent<PlayerStatusComponent>(statusEntity);
	if (!status) return;

	if (choiceName == "paddleSpread")
	{
		for (auto vausEntity : registry.View<VausTag, VausStateComponent>())
		{
			auto* vausState = registry.GetComponent<VausStateComponent>(vausEntity);
			vausState->widthScale += 0.05f;
		}
	}
	else if (choiceName == "hpLimitUp")
	{
		status->hpMax += 1;
	}
	else if (choiceName == "ballUp")
	{
		No::Vector3 spawnPos = No::Vector3::ZERO;
		bool foundLastBall = false;
		for (auto ve : registry.View<BallTag, No::TransformComponent>())
		{
			auto* tr = registry.GetComponent<No::TransformComponent>(ve);
			spawnPos = tr->translate;
			foundLastBall = true;
		}
		if (foundLastBall)
		{
			No::Vector3 dir = NoEngine::MathCalculations::Normalize(spawnPos);
			No::Vector3 ballPos = spawnPos + dir * 0.5f;

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
		status->ballCount++;
	}
	status->hp = status->hpMax;
	No::SoundPlay("upgrade", 1.0f,false);
	BallStateComponent::ballSlowFactor = BallStateComponent::kSlowRcp;
	BallStateComponent::ballSlowtime = 0.0f;
}