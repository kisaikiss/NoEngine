#include "stdafx.h"
#include "ShaderBackgroundPass.h"
#include "application/ClockworksDisease/Component/Background/ShaderBackgroundComponent.h"

ShaderBackgroundPass::Pipeline* ShaderBackgroundPass::GetOrCreate(const std::string& psPath) {
	auto it = pipelines_.find(psPath);
	if (it != pipelines_.end()) return it->second.get();

	// ShaderModuleはファイルが無いと last_write_time で例外を投げるため先に確認する
	if (!std::filesystem::exists(psPath)) {
		LogWarning("ShaderBackground: shader not found : " + psPath);
		pipelines_[psPath] = nullptr; // 毎フレーム警告しない
		return nullptr;
	}

	NoEngine::ShaderModule vs(NoEngine::ShaderStage::Vertex, L"resources/game/ClockworksDisease/Shaders/ShaderBackground.VS.hlsl", L"vs_6_0");
	NoEngine::ShaderModule ps(NoEngine::ShaderStage::Pixel, NoEngine::ConvertString(psPath), L"ps_6_0");

	auto p = std::make_unique<Pipeline>();
	p->rootSigName = "ShaderBackground : " + psPath;

	std::vector<NoEngine::ShaderReflection> refls{ vs.GetReflection(), ps.GetReflection() };
	NoEngine::RootSignatureBuilder::BuildFromReflection(refls, p->rootSig, p->rootSigName);

	D3D12_RASTERIZER_DESC rasterizer{};
	rasterizer.CullMode = D3D12_CULL_MODE_NONE;
	rasterizer.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blend{};
	blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 深度テストのみ(書き込みなし)。不透明メッシュの後ろにだけ描かれる
	D3D12_DEPTH_STENCIL_DESC depth{};
	depth.DepthEnable = true;
	depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	DXGI_FORMAT rtv[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	p->pso.SetRootSignature(p->rootSig);
	p->pso.SetRasterizerState(rasterizer);
	p->pso.SetBlendState(blend);
	p->pso.SetDepthStencilState(depth);
	p->pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	p->pso.SetRenderTargetFormats(1, rtv, DXGI_FORMAT_D24_UNORM_S8_UINT);
	p->pso.SetVertexShader(vs.GetBytecode());
	p->pso.SetPixelShader(ps.GetBytecode());
	p->pso.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	p->pso.Finalize(); // 入力レイアウトは設定しない(SV_VertexIDのみ。FullScreenと同じ)

	Pipeline* result = p.get();
	pipelines_[psPath] = std::move(p);
	return result;
}

void ShaderBackgroundPass::Execute(NoEngine::GraphicsContext& gfx, const NoEngine::RenderGraphRegistry& resourceRegistry, No::Registry& registry) {
	static_cast<void>(resourceRegistry);
	No::CameraComponent* camera = GetTargetCamera();
	if (!camera) return;
	auto* cameraTransform = registry.GetComponent<No::TransformComponent>(camera->entity);
	if (!cameraTransform) return;

	for (auto e : registry.View<ShaderBackgroundComponent, No::TransformComponent>()) {
		auto* bg = registry.GetComponent<ShaderBackgroundComponent>(e);
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		if (!bg->isVisible || bg->shaderPath.empty()) continue;

		Pipeline* pl = GetOrCreate(bg->shaderPath);
		if (!pl) continue;

		gfx.SetPipelineState(pl->pso);
		gfx.SetRootSignature(pl->rootSig);
		gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto& rootIndex = NoEngine::RootSignatureBuilder::GetRootIndexMap(pl->rootSigName);

		// 背景板をカメラの前方へ置き、カメラと同じ向きに回転させる。
		// 板のアスペクト比を保ったまま視野全体を覆う大きさにする。
		const float distance = camera->farClip * 0.95f;
		const No::Vector3 cameraPosition = cameraTransform->GetWorldPosition(registry);
		const No::Quaternion cameraRotation = cameraTransform->GetWorldRotation(registry);
		const No::Vector3 forward = cameraRotation.RotateVector(No::Vector3::FORWARD);
		const No::Vector3 worldScale = transform->GetWorldScale(registry);
		const float boardAspect = (std::fabs(worldScale.y) > 1e-5f)
			? std::fabs(worldScale.x / worldScale.y) : camera->aspect;
		const float viewHeight = 2.0f * distance * std::tan(camera->fov * 0.5f);
		const float viewWidth = viewHeight * camera->aspect;
		const float boardHeight = (boardAspect > 1e-5f)
			? (std::max)(viewHeight, viewWidth / boardAspect) : viewHeight;
		No::Vector3 backgroundScale = worldScale;
		backgroundScale.x = boardHeight * boardAspect;
		backgroundScale.y = boardHeight;
		No::Matrix4x4 world;
		world.MakeAffine(backgroundScale, cameraRotation, cameraPosition + forward * distance);
		No::Matrix4x4 wvp = world * camera->forGPU.viewProjection;
		gfx.SetDynamicConstantBufferView(rootIndex["VSConst"], sizeof(wvp), &wvp);

		// PS: 解像度は板のアスペクト比から決め、見た目がスケールに依存しないようにする
		No::Vector3 s = worldScale;
		float aspect = (std::fabs(s.y) > 1e-5f) ? std::fabs(s.x / s.y) : 1.0f;

		_declspec(align(16)) struct {
			float resolution[2];
			float time;
			float pad;
			No::Color color;
			No::Vector4 param;
		} constants;
		constants.resolution[0] = 720.0f * aspect;
		constants.resolution[1] = 720.0f;
		constants.time = NoEngine::GameCore::GetElapsedTime() * bg->timeScale;
		constants.pad = 0.0f;
		constants.color = bg->color;
		constants.param = bg->param;
		gfx.SetDynamicConstantBufferView(rootIndex["BackgroundConst"], sizeof(constants), &constants);

		gfx.Draw(6);
	}
}
