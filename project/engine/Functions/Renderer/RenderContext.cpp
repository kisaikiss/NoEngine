#include "RenderContext.h"
#include "engine/Functions/ECS/Component/LightComponent.h"
#include "engine/Runtime/GraphicsCore.h"

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
			L"PointLights",
			pointLightNum,
			sizeof(PointLightForGPU),
			pointLightUpload
		);
	}


	gfx.CopyBufferRegion(pointLightBuffer_, 0, pointLightUpload, 0, sizeof(PointLightForGPU) * pointLightNum);

}

void RenderContext::SetSpotLight(GraphicsContext& gfx, UploadBuffer& spotLightUpload, uint32_t spotLightNum) {
	bool recreate = false;
	if (lightNums_.spotLightNum != spotLightNum) recreate = true;
	if (spotLightNum == 0) return;

	if (recreate) {
		lightNums_.spotLightNum = spotLightNum;
		spotLightBuffer_.Create(
			L"SpotLights",
			spotLightNum,
			sizeof(SpotLightForGPU),
			spotLightUpload
		);
	}


	gfx.CopyBufferRegion(spotLightBuffer_, 0, spotLightUpload, 0, sizeof(SpotLightForGPU) * spotLightNum);
}

void RenderContext::CreateRaytraceInstanceBuffer(D3D12_HEAP_PROPERTIES& uploadHeap, CD3DX12_RESOURCE_DESC& instDesc) {
	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&instDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&instanceBuffer_)
	);
}

void RenderContext::CreateTLAS(CD3DX12_RESOURCE_DESC& tlasDesc) {
	D3D12_HEAP_PROPERTIES defaultHeap{};
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&tlasDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr,
		IID_PPV_ARGS(&tlas_)
	);
}

}
