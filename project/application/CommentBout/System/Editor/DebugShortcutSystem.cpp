#include "stdafx.h"
#include "DebugShortcutSystem.h"
#include "application/CommentBout/Component/Editor/DebugShortcutStateComponent.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

void DebugShortcutSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	DebugShortcutStateComponent* shortcut = nullptr;
	for (auto entity : registry.View<DebugShortcutStateTag, DebugShortcutStateComponent>()) {
		shortcut = registry.GetComponent<DebugShortcutStateComponent>(entity);
		break;
	}
	if (!shortcut) {
		return;
	}

#ifdef USE_IMGUI
	// LeftShift + 1 : カメラ切り替え
	if (No::Keyboard::IsPress(VK_LSHIFT) && No::Keyboard::IsTrigger('1')) {
		shortcut->useDebugCamera = !shortcut->useDebugCamera;
	}

	// LeftShift + 2 : デバッグ表示 全展開 / 全折り畳み
	if (No::Keyboard::IsPress(VK_LSHIFT) && No::Keyboard::IsTrigger('2')) {
		shortcut->debugDisplayAll = !shortcut->debugDisplayAll;
	}

	// LeftShift + 3 : 自機無敵トグル
	if (No::Keyboard::IsPress(VK_LSHIFT) && No::Keyboard::IsTrigger('3')) {
		shortcut->debugInvincible = !shortcut->debugInvincible;
	}

	// LeftShift + 4 : クリア/オーバー無効トグル
	if (No::Keyboard::IsPress(VK_LSHIFT) && No::Keyboard::IsTrigger('4')) {
		shortcut->debugDisableResult = !shortcut->debugDisableResult;
	}

	// デバッグオーバーレイ表示
	if (shortcut->debugInvincible || shortcut->debugDisableResult) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		if (viewport) {
			ImGui::SetNextWindowPos(
				ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 16.0f),
				ImGuiCond_Always,
				ImVec2(1.0f, 0.0f)
			);
		}
		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::Begin("DebugShortcutOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav);
		if (shortcut->debugInvincible) {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "自機無敵中");
		}
		if (shortcut->debugDisableResult) {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "クリアオーバー無効");
		}
		ImGui::End();
	}
#endif
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
