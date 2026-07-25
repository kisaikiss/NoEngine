#include "EditUtils.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/Command/CommandContext.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"
#include "DataDriven/PrefabSerializer.h"
#include "EditorCommandOperator.h"
#include "engine/Functions/Input/input.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {



namespace {
#ifdef USE_IMGUI
ImTextureID sGameTexture;
ImTextureID sSceneTexture;
Math::Vector2 sGameTextureSize = { 1280.f,720.f };
Math::Vector2 sSceneTextureSize = { 1280.f,720.f };
NoEngine::DescriptorHandle sGameTexSlot;  // スロットを保持しておく
NoEngine::DescriptorHandle sSceneTexSlot; // スロットを保持しておく
Math::Vector4 sSceneTexRect;
std::function<void(const Math::Vector4& imageRect)> sGizmoCallback;
bool sIsMouseOverWindow = false;
Math::Vector2 sGameWindowMousePosition{};
bool sIsMouseOverSceneWindow = false;
Math::Vector2 sSceneWindowMousePosition{};
#endif // USE_IMGUI
}


Math::Vector2 Editor::Get2DGameWindowMousePosition(ECS::Registry& registry) {
	Math::Vector2 result{};
#ifdef USE_IMGUI
	result = sGameWindowMousePosition;
	result -= GraphicsCore::GetWindowSize() / 2.f;
	auto cameraView = registry.View<Component::ActiveCamera2DTag>();
	for (auto e : cameraView) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);

		result = Math::Vector2(result.x * transform->scale.x, result.y * transform->scale.y);
		result += transform->translate;
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

	return result;
}

bool Editor::IsMouseOverGameWindow() {
#ifdef USE_IMGUI
	return sIsMouseOverWindow;
#else
	return false;
#endif // USE_IMGUI

}

Math::Vector2 Editor::Get2DSceneWindowMousePosition(ECS::Registry& registry) {
	Math::Vector2 result{};
#ifdef USE_IMGUI
	result = sSceneWindowMousePosition;
	result -= GraphicsCore::GetWindowSize() / 2.f;
	auto cameraView = registry.View<Component::ActiveCamera2DTag>();
	for (auto e : cameraView) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);

		result = Math::Vector2(result.x * transform->scale.x, result.y * transform->scale.y);
		result += transform->translate;
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

	return result;
}

bool Editor::IsMouseOverSceneWindow() {

#ifdef USE_IMGUI
	return sIsMouseOverSceneWindow;
#else
	return false;
#endif // USE_IMGUI
}



void Editor::DrawGrid2D(Math::Vector2 gridSize) {
	static const float kGridNum = 1000;
	for (uint32_t i = 0; i < kGridNum; i++) {
		DebugPrimitive::DrawLine2D(Math::Vector2(gridSize.x * i - gridSize.x * kGridNum / 2.f, -gridSize.x * kGridNum), Math::Vector2(gridSize.x * i - gridSize.x * kGridNum / 2.f, gridSize.x * kGridNum), Math::Color::WHITE);
		DebugPrimitive::DrawLine2D(Math::Vector2(-gridSize.x * kGridNum, gridSize.y * i - gridSize.y * kGridNum / 2.f), Math::Vector2(gridSize.x * kGridNum, gridSize.y * i - gridSize.y * kGridNum / 2.f), Math::Color::WHITE);
	}

}

std::string MakeUniqueName(const std::unordered_set<std::string>& used, const std::string& base) {
	if (used.find(base) == used.end()) return base;
	// base が既に使われている場合は base_1, base_2 ... を試す
	int i = 1;
	while (true) {
		std::string cand = base + "_" + std::to_string(i);
		if (used.find(cand) == used.end()) return cand;
		++i;
	}
}

std::unordered_set<std::string> CollectEditTagNames(ECS::Registry& registry, ECS::Entity except) {
	std::unordered_set<std::string> names;
	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		if (e == except) continue;
		const Editor::EditTag* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) names.insert(tag->name);
	}
	return names;
}

ECS::Entity PickObject(CommandContext& ctx, ColorBuffer& idColorBuffer, ReadbackBuffer& readBackBuffer) {
#ifdef USE_IMGUI

	static bool isPickObject = false;
	if (isPickObject) {
		const uint8_t* pixelData = reinterpret_cast<const uint8_t*>(readBackBuffer.Map());

		// DXGI_FORMAT_R8G8B8A8_UNORM はメモリ上では R, G, B, A の順で1バイトずつ格納されます
		uint8_t r = pixelData[0];
		uint8_t g = pixelData[1];
		uint8_t b = pixelData[2];
		uint8_t a = pixelData[3];

		readBackBuffer.Unmap();

		// RGB値からオブジェクトIDを復元する
		uint32_t pickedID = (r << 24) | (g << 16) | (b << 8) | a;
		constexpr uint32_t kNoObjectNumber = 1724754687;
		if (pickedID == kNoObjectNumber) {
			return ECS::INVALID_ENTITY;
		}

		LogDebug("Object click select ID : " + std::to_string(pickedID));
		isPickObject = false;
		return static_cast<ECS::Entity>(pickedID);
	}

	if (Input::Mouse::IsTrigger(Input::MouseButton::Left)) {
		if (!Editor::IsMouseOverSceneWindow()) {
			return ECS::INVALID_ENTITY;
		}

		ctx.CopyPixelToBuffer(readBackBuffer, idColorBuffer, static_cast<UINT>(sSceneWindowMousePosition.x), static_cast<UINT>(sSceneWindowMousePosition.y));
		isPickObject = true;
	}
	
	

#else
	static_cast<void>(ctx);
	static_cast<void>(idColorBuffer);
	static_cast<void>(readBackBuffer);

#endif // USE_IMGUI

	return ECS::INVALID_ENTITY;
}

void InitGameImGuiWindow(ColorBuffer& mainColor) {
#ifdef USE_IMGUI
	sGameTexSlot = Render::gTextureHeap.Alloc();
	sGameTexture = static_cast<ImTextureID>(sGameTexSlot.GetGpuPtr());
	RefreshGameImGuiTexture(mainColor);
#else
	static_cast<void>(mainColor);
#endif // USE_IMGUI
}

void InitSceneImGuiWindow(ColorBuffer& debugColor) {
#ifdef USE_IMGUI
	sSceneTexSlot = Render::gTextureHeap.Alloc();
	sSceneTexture = static_cast<ImTextureID>(sSceneTexSlot.GetGpuPtr());
	RefreshSceneImGuiTexture(debugColor);
#else
	static_cast<void>(debugColor);
#endif // USE_IMGUI
}

void RefreshGameImGuiTexture(ColorBuffer& mainColor) {
#ifdef USE_IMGUI
	GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
		1,
		static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(sGameTexSlot),
		mainColor.GetSRV(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// アスペクト比計算にも実サイズを反映する（要 ColorBuffer に GetWidth/GetHeight）
	sGameTextureSize = Math::Vector2(
		static_cast<float>(mainColor.GetWidth()),
		static_cast<float>(mainColor.GetHeight()));
#else
	static_cast<void>(mainColor);
#endif // USE_IMGUI
}

void RefreshSceneImGuiTexture(ColorBuffer& debugColor) {
#ifdef USE_IMGUI
	GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
		1,
		static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(sSceneTexSlot),
		debugColor.GetSRV(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	sSceneTextureSize = Math::Vector2(
		static_cast<float>(debugColor.GetWidth()),
		static_cast<float>(debugColor.GetHeight()));
#else
	static_cast<void>(debugColor);
#endif // USE_IMGUI
}

void DrawGameImGuiWindow() {
#ifdef USE_IMGUI
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	ImGui::Begin("Game");

	// パネルの実際の表示可能領域
	ImVec2 avail = ImGui::GetContentRegionAvail();

	// シーンテクスチャ本来の解像度からアスペクト比を求める
	float texWidth = sGameTextureSize.x;  // オフスクリーンRTの幅
	float texHeight = sGameTextureSize.y; // オフスクリーンRTの高さ
	float texAspect = texWidth / texHeight;

	ImVec2 imageSize;
	if (avail.x / avail.y > texAspect) {
		// 利用可能領域が横長 → 高さ基準（左右に余白）
		imageSize.y = avail.y;
		imageSize.x = avail.y * texAspect;
	} else {
		// 利用可能領域が縦長 → 幅基準（上下に余白）
		imageSize.x = avail.x;
		imageSize.y = avail.x / texAspect;
	}

	// 中央寄せ
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 centeredOffset(
		(avail.x - imageSize.x) * 0.5f,
		(avail.y - imageSize.y) * 0.5f
	);
	ImGui::SetCursorPos(ImVec2(cursorPos.x + centeredOffset.x, cursorPos.y + centeredOffset.y));

	ImGui::Image(sGameTexture, imageSize);

	sIsMouseOverWindow = false;
	if (ImGui::IsItemHovered()) {
		ImVec2 imgPos = ImGui::GetItemRectMin();
		ImVec2 imgSize = ImGui::GetItemRectSize();
		ImVec2 mouse = ImGui::GetMousePos();

		ImVec2 local(mouse.x - imgPos.x, mouse.y - imgPos.y);

		if (0 <= local.x && local.x < imgSize.x &&
			0 <= local.y && local.y < imgSize.y) {
			ImVec2 uv(local.x / imgSize.x, local.y / imgSize.y);

			// ウィンドウ内のスクリーン座標に変換
			sGameWindowMousePosition.x = uv.x * windowSize.x;
			sGameWindowMousePosition.y = uv.y * windowSize.y;
			sIsMouseOverWindow = true;
		}
	}

	ImGui::End();
#endif // USE_IMGUI
}
void DrawSceneImGuiWindow(ECS::Registry& registry, CommandContext& ctx, ColorBuffer& worldPositionColorBuffer, ReadbackBuffer& readBackBuffer) {
#ifdef USE_IMGUI

	static_cast<void>(ctx);
	static_cast<void>(worldPositionColorBuffer);
	static_cast<void>(readBackBuffer);

	ImGui::Begin("Scene");

	// パネルの実際の表示可能領域
	ImVec2 avail = ImGui::GetContentRegionAvail();

	// シーンテクスチャ本来の解像度からアスペクト比を求める
	float texWidth = sSceneTextureSize.x;  // オフスクリーンRTの幅
	float texHeight = sSceneTextureSize.y; // オフスクリーンRTの高さ
	float texAspect = texWidth / texHeight;

	ImVec2 imageSize;
	if (avail.x / avail.y > texAspect) {
		// 利用可能領域が横長 → 高さ基準（左右に余白）
		imageSize.y = avail.y;
		imageSize.x = avail.y * texAspect;
	} else {
		// 利用可能領域が縦長 → 幅基準（上下に余白）
		imageSize.x = avail.x;
		imageSize.y = avail.x / texAspect;
	}

	// 中央寄せ
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 centeredOffset(
		(avail.x - imageSize.x) * 0.5f,
		(avail.y - imageSize.y) * 0.5f
	);
	ImGui::SetCursorPos(ImVec2(cursorPos.x + centeredOffset.x, cursorPos.y + centeredOffset.y));

	ImGui::Image(sSceneTexture, imageSize);

	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	sSceneTexRect = Math::Vector4(min.x, min.y, max.x - min.x, max.y - min.y);

	Math::Vector2 windowSize(imageSize.x, imageSize.y); // 以降の座標変換は実表示サイズ基準にする

	sIsMouseOverSceneWindow = false;
	if (ImGui::IsItemHovered()) {
		ImVec2 imgPos = ImGui::GetItemRectMin();
		ImVec2 imgSize = ImGui::GetItemRectSize();
		ImVec2 mouse = ImGui::GetMousePos();

		ImVec2 local(mouse.x - imgPos.x, mouse.y - imgPos.y);

		if (0 <= local.x && local.x < imgSize.x &&
			0 <= local.y && local.y < imgSize.y) {
			ImVec2 uv(local.x / imgSize.x, local.y / imgSize.y);
			sSceneWindowMousePosition.x = uv.x * texWidth;   // UV→テクスチャ実解像度で変換
			sSceneWindowMousePosition.y = uv.y * texHeight;
			sIsMouseOverSceneWindow = true;
		}
	}

	static ECS::Entity instantiateTransformObjectEntity = ECS::INVALID_ENTITY;
	if (instantiateTransformObjectEntity != ECS::INVALID_ENTITY) {
		registry.AddComponent<Editor::EditSelectedTag>(instantiateTransformObjectEntity);
		const float* pixelData = reinterpret_cast<const float*>(readBackBuffer.Map());

		float r = pixelData[0];
		float g = pixelData[1];
		float b = pixelData[2];

		readBackBuffer.Unmap();

		const Math::Color clearColor = worldPositionColorBuffer.GetClearColor();
		if (r != clearColor.r || g != clearColor.g || b != clearColor.b) {
			auto& translate = registry.GetComponent<Component::TransformComponent>(instantiateTransformObjectEntity)->translate;
			translate = Math::Vector3(r, g, b);
		}

		instantiateTransformObjectEntity = ECS::INVALID_ENTITY;
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB_PATH")) {
			const char* path = (const char*)payload->Data;
			// インスタンス化
			auto e = Editor::InstantiatePreset(registry, std::string(path));
			registry.AddComponent<Editor::EditSelectedTag>(e);

			// Transform があるなら
			if (auto* t = registry.GetComponent<Component::TransformComponent>(e)) {
				ctx.CopyPixelToBuffer(readBackBuffer, worldPositionColorBuffer,
					static_cast<UINT>(sSceneWindowMousePosition.x), static_cast<UINT>(sSceneWindowMousePosition.y), DXGI_FORMAT_R32G32B32A32_TYPELESS);

				// カメラの向いている方向から50m離れた場所に配置
				auto cameraView = registry.View<Component::CameraComponent, Component::DebugCameraComponent>();
				Component::CameraComponent* camera = nullptr;
				Component::TransformComponent* transform = nullptr;
				for (auto entity : cameraView) {
					camera = registry.GetComponent<Component::CameraComponent>(entity);
					transform = registry.GetComponent<Component::TransformComponent>(entity);
				}
				if (camera && transform) {
					Math::Vector3 worldPos = camera->forGPU.worldPosition;
					const float kOffset = 50.f;
					worldPos += transform->rotation.zAxis() * kOffset;
					t->translate = worldPos;
				}
				instantiateTransformObjectEntity = e;
			}
			// Undo 用コマンド登録
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		ImGui::EndDragDropTarget();
	}

	if (sGizmoCallback) sGizmoCallback(sSceneTexRect);
	ImGui::End();
#else
	static_cast<void>(registry);
	static_cast<void>(ctx);
	static_cast<void>(worldPositionColorBuffer);
	static_cast<void>(readBackBuffer);
#endif // USE_IMGUI
}

void SetGizmoCallback(std::function<void(const Math::Vector4& imageRect)> cb) {
#ifdef USE_IMGUI
	sGizmoCallback = cb;
#else
	static_cast<void>(cb);
#endif // USE_IMGUI

}

}

