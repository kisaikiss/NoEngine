#include "VausControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhysicsComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/GraphicsCore.h"

using namespace NoEngine;

constexpr float kRingRadius = 4.85f;
constexpr float kChargeSpeed = 2.5f; // charge per second
constexpr float kMaxCharge = 8.0f; // max launch speed

void VausControlSystem::Update(No::Registry& registry, float deltaTime)
{
	(void)deltaTime;
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
		No::MaterialComponent>();

	float angle = CalculateMouseAngle();
	static float currentCharge = 0.0f;
	bool isPress = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	static bool wasPress = false;

	for (auto entity : vausView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		// リングの円周上にXY平面の角度で配置
		Vector3 ringPos{ kRingRadius * std::cos(angle), kRingRadius * std::sin(angle),-0.25f };
		transform->translate = ringPos;

		transform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);

		//リングの中心からデバッグラインを描画
#ifdef _DEBUG
		auto normal = MathCalculations::Normalize(Vector3::ZERO - ringPos);
		Primitive::DrawLine((ringPos + normal), ringPos, Color(0.2f, 0.2f, 0.8f, 1.0f));
#endif



	}
	for (auto entity : ringView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);


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