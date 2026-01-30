#include "BallTrailPass.h"
#include "application/TD2_3/Component/BallTrailComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Easing.h"

#include <algorithm>

using namespace NoEngine;
using namespace NoEngine::Render;

BallTrailPass::BallTrailPass() {}
BallTrailPass::~BallTrailPass() {}

void BallTrailPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
    // 収集して頂点生成
    CollectAndGenerate(registry);

    if (vertices_.empty() || indices_.empty()) return;

    // レンダリング
    std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Ball Trail PSO");
    gfx.SetPipelineState(GetPSO(Render::GetPSOID(L"Renderer : Ball Trail PSO")));
    gfx.SetRootSignature(GetRootSignature(Render::GetRootSignatureID(L"Renderer : Ball Trail PSO")));
    gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // カメラ行列
    gfx.SetDynamicConstantBufferView(rootIndex["gCamera"], sizeof(Matrix4x4), &GetCamera()->GetViewProjMatrix());

    // マテリアル色（透明含む）
    _declspec(align(16)) struct
    { 
		float maxAge;
		float pad[3];
    } matConst;
	matConst.maxAge = 0.6f;

    gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(matConst), &matConst);

    // 頂点・インデックスをバインドして描画
    gfx.SetDynamicVB(0, vertices_.size(), sizeof(TrailVertex), vertices_.data());
    gfx.SetDynamicIB(indices_.size(), indices_.data());

    gfx.DrawIndexedInstanced(static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
}

void BallTrailPass::CollectAndGenerate(ECS::Registry& registry) {
    vertices_.clear();
    indices_.clear();

    auto view = registry.View<
        NoEngine::Component::TransformComponent,
        BallTrailComponent
    >();

    if (view.Empty()) return;

    for (auto entity : view) {
        auto* trail = registry.GetComponent<BallTrailComponent>(entity);

        // 既にワールド位置で記録している前提。もし Transform が Z 高さを持つならそのまま使える。
        if (trail->samples.size() < 2) continue;

        // サンプル列から頂点・インデックスを作成する（各サンプルで左右頂点 = 2頂点）
        size_t baseIndex = vertices_.size();
        const auto& s = trail->samples;
        size_t n = s.size();

        // CPU側で法線（XY平面）を計算して左右頂点を生成
        for (size_t i = 0; i < n; ++i) {
            // 隣接サンプルを使って接線を計算
            NoEngine::Vector3 p = s[i].pos;
            NoEngine::Vector3 dir;
            if (i == 0) dir = (n > 1) ? (s[0].pos - s[1].pos) : NoEngine::Vector3(1,0,0);
            else if (i == n - 1) dir = (s[n - 1].pos - s[n - 2].pos);
            else dir = (s[i - 1].pos - s[i + 1].pos);

            dir.z = 0.0f;
            dir = MathCalculations::Normalize(dir);
            // 法線（XY平面）
            NoEngine::Vector3 normal = MathCalculations::Normalize(NoEngine::Vector3(-dir.y, dir.x, 0.0f));

            float ageRatio = std::clamp(s[i].age / trail->maxAge, 0.0f, 1.0f);
            float localThickness = trail->thickness * (1.0f - ageRatio);

            NoEngine::Vector3 leftPos = p + normal * (localThickness * 0.5f);
            NoEngine::Vector3 rightPos = p - normal * (localThickness * 0.5f);

            NoEngine::Color vertexColor;
            float tNorm = ageRatio; // 0..1 (若い=0 -> ヘッド)
            if (trail->useSegmentedColors && trail->colorSegments > 0)
            {
                int segCount = static_cast<int>(trail->colorSegments);
                float segSize = 1.0f / static_cast<float>(segCount);
                int segIdx = std::min(static_cast<int>(tNorm / segSize), segCount - 1);
                float segStart = segIdx * segSize;
                float segEnd = std::min((segIdx + 1) * segSize, 1.0f);
                float localT = (segEnd - segStart) > 0.0f ? ((tNorm - segStart) / (segEnd - segStart)) : 0.0f;
                NoEngine::Color cStart = Easing::Lerp(trail->startColor, trail->endColor, segStart);
                NoEngine::Color cEnd = Easing::Lerp(trail->startColor, trail->endColor, segEnd);
                vertexColor = Easing::EaseOutCirc(cStart, cEnd, localT);
            }
            else
            {
                // 連続グラデーション（フォールバック）
                vertexColor = Easing::EaseOutCirc(trail->startColor, trail->endColor, tNorm);
            }

            TrailVertex vLeft;
            vLeft.position = NoEngine::Vector4(leftPos.x, leftPos.y, leftPos.z, 1.0f);
            vLeft.color = vertexColor;
            vLeft.age = s[i].age;
            vertices_.push_back(vLeft);

            TrailVertex vRight;
            vRight.position = NoEngine::Vector4(rightPos.x, rightPos.y, rightPos.z, 1.0f);
            vRight.color = vertexColor;
            vRight.age = s[i].age;
            vertices_.push_back(vRight);
        }

        // インデックス（各サンプルで四角を二つの三角形に分ける）
        for (size_t i = 0; i + 1 < n; ++i) {
            uint16_t i0 = static_cast<uint16_t>(baseIndex + i * 2 + 0);
            uint16_t i1 = static_cast<uint16_t>(baseIndex + i * 2 + 1);
            uint16_t i2 = static_cast<uint16_t>(baseIndex + (i + 1) * 2 + 0);
            uint16_t i3 = static_cast<uint16_t>(baseIndex + (i + 1) * 2 + 1);

            // tri1: i0, i2, i1
            indices_.push_back(i0);
            indices_.push_back(i2);
            indices_.push_back(i1);
            // tri2: i1, i2, i3
            indices_.push_back(i1);
            indices_.push_back(i2);
            indices_.push_back(i3);
        }
    }
}