#include "RenderContext.h"
#include "engine/Functions/ECS/Component/LightComponent.h"

namespace NoEngine {
using namespace Component;
void RenderContext::SetDirectionalLight(GraphicsContext& gfx, UploadBuffer& directionalLightUpload, uint32_t directionalLightNum) {
	bool recreate = false;
	if (lightNums_.directionalLightNum != directionalLightNum) recreate = true;
	if (directionalLightNum == 0) return;
	
	if (recreate) {
		lightNums_.directionalLightNum = directionalLightNum;
		directionalLightBuffer_.Create(
			L"DirectionalLights",
			directionalLightNum,
			sizeof(DirectionalLightComponent),
			directionalLightUpload
		);
	}
	

	gfx.CopyBufferRegion(directionalLightBuffer_, 0, directionalLightUpload, 0, sizeof(DirectionalLightComponent) * directionalLightNum);

}

void RenderContext::SetPointLight(GraphicsContext& gfx, UploadBuffer& pointLightUpload, uint32_t pointLightNum) {
	bool recreate = false;
	if (lightNums_.pointLightNum != pointLightNum) recreate = true;
	if (pointLightNum == 0) return;

	if (recreate) {
		lightNums_.pointLightNum = pointLightNum;
		pointLightBuffer_.Create(
			L"DirectionalLights",
			pointLightNum,
			sizeof(PointLightForGPU),
			pointLightUpload
		);
	}


	gfx.CopyBufferRegion(pointLightBuffer_, 0, pointLightUpload, 0, sizeof(PointLightForGPU) * pointLightNum);

}

}
