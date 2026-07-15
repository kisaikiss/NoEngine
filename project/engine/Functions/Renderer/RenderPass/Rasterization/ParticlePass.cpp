#include "stdafx.h"
#include "ParticlePass.h"
#include "engine/Functions/Particle/ParticleManager.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"

namespace NoEngine {
namespace Render {
struct ParticleVertex {
	Math::Vector4 position;
	Math::Vector2 texcoord;
};

ParticlePass::ParticlePass() {
	vertex_ = ByteAddressBuffer();
	ParticleVertex vertices[] = {
		{{-0.5f,-0.5f,0.f,1.f},{0,1}},
		{{0.5f,-0.5f,0.f,1.f},{1,1}},
		{{-0.5f,0.5f,0.f,1.f},{0,0}},
		{{0.5f,0.5f,0.f,1.f},{1,0}},
	};

	UploadBuffer vertexUpload;
	vertexUpload.Create(L"VertexUpload", sizeof(vertices));
	memcpy(vertexUpload.Map(), vertices, sizeof(vertices));
	vertexUpload.Unmap();
	vertex_.Create(L"partilceVertex", 4, sizeof(ParticleVertex), vertexUpload);

	uint32_t indices[] = {
		0,1,2,1,3,2
	};
	UploadBuffer indexUpload;
	indexUpload.Create(L"IndexUpload", sizeof(indices));
	memcpy(indexUpload.Map(), indices, sizeof(indices));
	indexUpload.Unmap();
	index_.Create(L"particleIndex", sizeof(indices), sizeof(uint32_t), indexUpload);

	testTexture_ = TextureManager::LoadCovertTexture("resources/engine/Texture/circle.png");
}

void ParticlePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;
	camera_ = GetTargetCamera();
	if (!camera_) return;
	auto renderCtx = GetRenderContext();

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : particle PSO");
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : particle PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : particle PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	__declspec(align(16))struct {
		Math::Matrix4x4 viewProjectionMatrix;
		Math::Matrix4x4 billBoardMatrix;
	}parViewConstants;
	parViewConstants.viewProjectionMatrix = camera_->forGPU.viewProjection;
	Math::Matrix4x4 backToFrontMatrix;
	backToFrontMatrix.MakeRotate(Math::Vector3::UP* PI);
	auto* transform = registry.GetComponent<Component::TransformComponent>(GetTargetCamera()->entity);
	parViewConstants.billBoardMatrix = backToFrontMatrix * transform->MakeAffineMatrix4x4(registry);
	parViewConstants.billBoardMatrix.m[3][0] = 0.0f;
	parViewConstants.billBoardMatrix.m[3][1] = 0.0f;
	parViewConstants.billBoardMatrix.m[3][2] = 0.0f;

	gfx.SetDynamicConstantBufferView(rootIndex["gParView"], sizeof(parViewConstants), &parViewConstants);

	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, testTexture_->GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gParticles"], 0, ParticleManager::GetParticleBuffer().GetSRV());
	gfx.SetVertexBuffer(0, vertex_.VertexBufferView());
	gfx.SetIndexBuffer(index_.IndexBufferView());
	gfx.DrawIndexedInstanced(6, ParticleManager::GetParticleNum(), 0, 0, 0);
}

}
}