#include "VausControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/RingAnimationComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/BallStateComponent.h"

#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Easing.h"
#include "externals/imgui/imgui.h"

using namespace NoEngine;

namespace
{
	constexpr float kRingInitialRadius = 4.85f;
	float sRingRadius = kRingInitialRadius;
}

void VausControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto vausView = registry.View<
		VausTag,
		No::TransformComponent,
		No::MaterialComponent,
		VausStateComponent>();


	auto ballView = registry.View<
		BallTag,
		No::TransformComponent,
		No::MaterialComponent>();

	auto ringView = registry.View<
		RingTag,
		No::TransformComponent,
		No::MaterialComponent,
		RingAnimationComponent>();
	
	float angle = CalculateMouseAngle();
	static float currentCharge = 0.0f;
	isPress_ = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	static float chargeTime = 0.0f;
	static float power = 0.0f;
	for (auto entity : ringView)
	{
		auto* ringTrans = registry.GetComponent<No::TransformComponent>(entity);
		auto* ringAnimation = registry.GetComponent<RingAnimationComponent>(entity);
		if (isPress_)
		{
			if (!wasPress_)
			{
				power = 0.0f;
			}
			ringAnimation->releaseTime = 0.0f;
			ringAnimation->pressedTime += deltaTime * 2.0f;
			ringAnimation->pressedTime = std::clamp(ringAnimation->pressedTime, 0.0f, 1.0f);
			chargeTime = Easing::Lerp(0.0f, 1.0f, ringAnimation->pressedTime);
			ringAnimation->tTemp = chargeTime;
			if (chargeTime >= 1.0f)
			{
				ringTrans->translate.x += static_cast<float>( rand() % 3 - 1) * deltaTime;
				ringTrans->translate.y += static_cast<float>(rand() % 3 - 1) * deltaTime;
			}
		}
		else
		{
			ringTrans->translate = Vector3::ZERO;
			if (wasPress_ && !isPress_)
			{
				power = ringAnimation->tTemp;
			}
			ringAnimation->pressedTime = 0.0f;
			ringAnimation->releaseTime += deltaTime;
			ringAnimation->releaseTime = std::clamp(ringAnimation->releaseTime, 0.0f, 1.0f);
			chargeTime = Easing::EaseOutElastic(ringAnimation->tTemp, 0.0f, ringAnimation->releaseTime);
		}
		ringTrans->scale = Vector3::UNIT_SCALE * (1.0f + ringAnimation->kChargeScale * chargeTime);
		sRingRadius = kRingInitialRadius + chargeTime;
	}
	if (!wasPress_ && isPress_)
	{
		for (auto entity : ballView)
		{
			auto* ballState = registry.GetComponent<BallStateComponent>(entity);
			auto* ballPhysics = registry.GetComponent<PhysicsComponent>(entity);
			if (ballState)
			{
				ballState->landed = false;
			}
			if (ballPhysics)
			{
				ballPhysics->useGravity = true;
			}
		}
	}
	for (auto entity : vausView)
	{
		auto* vausTransform = registry.GetComponent<No::TransformComponent>(entity);
		auto* vausState = registry.GetComponent<VausStateComponent>(entity);
		Vector3 posBeforeUpdate = vausTransform->translate;
		vausState->currentRingRadius = sRingRadius;
		vausState->chargePower = power;
		vausState->theta = angle;
		Vector3 ringPos{ sRingRadius * std::cos(angle), sRingRadius * std::sin(angle), -0.25f };
		vausTransform->translate = ringPos;
		vausTransform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);
		vausTransform->scale = Vector3::UNIT_SCALE * (1.0f + 0.2f * chargeTime);
		vausState->isReleasing = false;
		ImGui::Text("RingRadius: %.3f", vausState->currentRingRadius);
		if (wasPress_ && !isPress_)
		{
			vausState->isReleasing = true;
		}
		if (!isPress_ && chargeTime > 0.001f)
		{
			vausState->isReleasing = true;
		}

		if (deltaTime > 0.0f)
		{
			vausState->currentVelocity = (vausTransform->translate - vausState->prevPosition) / deltaTime;
		}
		else
		{
			vausState->currentVelocity = Vector3::ZERO;
		}

		vausState->prevPosition = vausTransform->translate;
	}

	wasPress_ = isPress_;
}

float VausControlSystem::CalculateMouseAngle()
{

	POINT pt;
	if (GetCursorPos(&pt))
	{
		ScreenToClient(GraphicsCore::gWindowManager.GetMainWindow()->GetWindowHandle(), &pt);
	}

	//クライアント座標でウィンドウの中心を取得
	RECT rc;
	auto* main = GraphicsCore::gWindowManager.GetMainWindow();
	int cx = 0, cy = 0;
	if (main)
	{
		GetClientRect(main->GetWindowHandle(), &rc);
		cx = (rc.right - rc.left) / 2;
		cy = (rc.bottom - rc.top) / 2;
	}

	//画面空間で中心からマウスまでのベクトルを計算
	float mx = static_cast<float>(pt.x - cx);
	float my = static_cast<float>(-(pt.y - cy));

	return std::atan2f(my, mx);
}