#pragma once
#include "MeshPass.h"

namespace NoEngine {
namespace Render {

/// <summary>
/// 半透明メッシュのみを描画するレンダーパス。
/// DepthOfFieldPass適用後(パーティクルと同様の合成タイミング)に実行することで、
/// 半透明オブジェクトがDoFのボケの影響を受けないようにする。
/// MainDepthは既存の不透明の深度に対する深度テストのみに使用し、書き込み・クリアは行わない。
/// </summary>
class TransparentMeshPass :
    public MeshPassBase {
public:
    /// <summary>
    /// 半透明メッシュ描画を実行する
    /// </summary>
    /// <param name="gfx">描画用コマンドリストのラッパークラスの参照</param>
    /// <param name="registry">ECSレジストリ</param>
    virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
};
}
}