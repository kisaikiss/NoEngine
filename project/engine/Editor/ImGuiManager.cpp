#include "ImGuiManager.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Renderer/RenderSystem.h"

// ToDo : ImGuiは現在NoEngine.sln内に配置しています。専用のImGui.slnを作成し、そこに配置すべきです。
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

namespace NoEngine {

void ImGuiManager::Initialize() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// ToDo : ImGuiウィンドウを画面外に出せるように設定しましょう。
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImFontConfig config;
	config.OversampleH = 2;
	config.OversampleV = 2;
	config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF("externals/fonts/ISE-FONT4/Isego.otf", 14.0f, &config);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.FramePadding = ImVec2(12, 6);
	style.ItemSpacing = ImVec2(10, 8);
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 6.0f;

	
	

	ImVec4* colors = style.Colors;
	// 基本色
	colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(1.f, 0.0f, 1.0f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImColor(25, 175, 85, 255);
	colors[ImGuiCol_ChildBg] = ImColor(1.00f, 0.98f, 0.99f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImColor(1.00f, 0.90f, 0.95f, 1.00f);


	colors[ImGuiCol_TitleBg] = ImColor(150, 0, 100, 255);
	colors[ImGuiCol_TitleBgActive] = ImColor(230, 33, 255, 255);
	colors[ImGuiCol_MenuBarBg] = ImColor(185, 120, 255, 255);

	// 枠線・影
	colors[ImGuiCol_Border] = ImVec4(0.90f, 0.70f, 0.80f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImColor(25, 175, 85, 255);

	// フレーム系
	colors[ImGuiCol_FrameBg] = ImColor(233, 12, 122, 255);
	colors[ImGuiCol_FrameBgHovered] = ImColor(240, 50, 180, 255);
	colors[ImGuiCol_FrameBgActive] = ImColor(255, 100, 200, 255);

	// ボタン
	colors[ImGuiCol_Button] = ImColor(0, 110, 110, 255);
	colors[ImGuiCol_ButtonHovered] = ImColor(0, 150, 80, 255);
	colors[ImGuiCol_ButtonActive] = ImColor(0, 60, 40, 255);

	// タブ
	colors[ImGuiCol_Tab] = ImColor(130, 0, 140, 255);
	colors[ImGuiCol_TabHovered] = ImColor(255, 55, 155, 255);
	colors[ImGuiCol_TabActive] = ImColor(255, 0, 130, 255);
	colors[ImGuiCol_TabUnfocused] = ImColor(40, 0, 100, 255);
	colors[ImGuiCol_TabUnfocusedActive] = ImColor(100, 20, 200, 255);

	// スライダー
	colors[ImGuiCol_SliderGrab] = ImColor(255, 120, 210, 255);
	colors[ImGuiCol_SliderGrabActive] = ImColor(80, 0, 55, 255);

	// ヘッダー・チェックマーク
	colors[ImGuiCol_Header] = ImColor(220, 0, 190, 255);
	colors[ImGuiCol_HeaderHovered] = ImColor(165, 0, 100, 255);
	colors[ImGuiCol_HeaderActive] = ImColor(70, 0, 50, 255);
	colors[ImGuiCol_CheckMark] = ImColor(0, 150, 80, 255);

#pragma region "keyboardColors"

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.66f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.95f);
	colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.01f, 0.02f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 0.62f, 0.00f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.62f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.06f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_Header] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.03f, 0.03f, 0.02f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.04f, 0.03f, 0.02f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 0.80f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.85f, 0.70f, 0.40f, 0.90f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.94f, 0.44f, 0.05f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	// スタイル設定
	style.WindowRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 4.0f;
	style.FrameBorderSize = 1.0f;
#pragma endregion



	Window* window = GraphicsCore::gWindowManager.GetMainWindow();
	ImGui_ImplWin32_Init(window->GetWindowHandle());

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = GraphicsCore::gGraphicsDevice->GetDevice();
	initInfo.CommandQueue = GraphicsCore::gCommandListManager.GetGraphicsQueue().GetCommandQueue();
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
	initInfo.SrvDescriptorHeap = Render::gTextureHeap.GetHeapPointer();
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
		auto& textureHeap = Render::gTextureHeap;
		DescriptorHandle handle = textureHeap.Alloc();

		*out_cpu_handle = D3D12_CPU_DESCRIPTOR_HANDLE(handle);
		*out_gpu_handle = D3D12_GPU_DESCRIPTOR_HANDLE(handle);
		};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {cpu_handle; gpu_handle; };
	ImGui_ImplDX12_Init(&initInfo);
}

void ImGuiManager::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("MainDockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

}

void ImGuiManager::Render(GraphicsContext& context) {
	ImGui::Render();
	context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Render::gTextureHeap.GetHeapPointer());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
}

void ImGuiManager::Shutdown() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

}
