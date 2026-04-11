#pragma once
#include "RenderPass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/ECS/Component/LightComponent.h"
#include "../LightForGPU.h"

namespace NoEngine {
namespace Render {
/// <summary>
/// 描画に必要なライトをGPUへアップロードするパス
/// </summary>
class LightPass : public RenderPass {
public:
	/// <summary>
	/// ライトのアップロードを実行する
	/// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラス</param>
	/// <param name="registry">ECSレジストリ</param>
	void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;

private:
	/// <summary>
	/// ライトのアップロードに必要なものを収集する
	/// </summary>
	/// <param name="registry">ECSレジストリ</param>
	void Collect(ECS::Registry& registry);

	/// <summary>
	/// GPUへライトをアップロードする
	/// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラス</param>
	void UploadToGpu(GraphicsContext& gfx);

	// Lightのアップロードバッファ
	UploadBuffer directionalLightUpload_;
	UploadBuffer pointLightUpload_;
	UploadBuffer spotLightUpload_;

	// Registryから収集したLightの配列
	std::vector<Component::DirectionalLightComponent> directionalLights_;
	std::vector<PointLightForGPU> pointLights_;
	std::vector<SpotLightForGPU> spotLights_;
	// Light配列のサイズ
	size_t directionalLightsSize_;
	size_t pointLightsSize_;
	size_t spotLightsSize_;
};
}
}

