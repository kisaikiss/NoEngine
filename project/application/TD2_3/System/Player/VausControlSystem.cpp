#include "VausControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/RingAnimationComponent.h"
#include "../../Component/VausStateComponent.h"

#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Easing.h"

using namespace NoEngine;

constexpr float kRingRadius = 4.85f;
void VausControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto vausView = registry.View<
		VausTag,
		No::TransformComponent,
		No::MaterialComponent>();

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
	bool isPress = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	static bool wasPress = false;

	for (auto entity : ringView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* ringAnimation = registry.GetComponent<RingAnimationComponent>(entity);
		if (isPress)
		{
			ringAnimation->releaseTime = 0.0f;
			ringAnimation->pressedTime += deltaTime * 2.0f;
			ringAnimation->pressedTime = std::clamp(ringAnimation->pressedTime, 0.0f, 1.0f);
			transform->scale = Easing::Lerp(ringAnimation->baseScale, ringAnimation->targetScale, ringAnimation->pressedTime);
		}
		else
		{
			ringAnimation->pressedTime = 0.0f;
			ringAnimation->releaseTime += deltaTime;
			ringAnimation->releaseTime = std::clamp(ringAnimation->releaseTime, 0.0f, 1.0f);
			transform->scale = Easing::EaseOutElastic(ringAnimation->targetScale, ringAnimation->baseScale, ringAnimation->releaseTime);
		}
	}

	for (auto entity : vausView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		Vector3 ringPos{ kRingRadius * std::cos(angle), kRingRadius * std::sin(angle), -0.25f };
		transform->translate = ringPos;
		transform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);

	}

	wasPress = isPress;
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