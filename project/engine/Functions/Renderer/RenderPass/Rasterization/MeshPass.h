#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"

namespace NoEngine {
namespace Render {

/// <summary>
/// MeshPass / TransparentMeshPass が共有するメッシュ描画の基底クラス。
/// </summary>
class MeshPassBase :
    public RenderPass {
public:
    MeshPassBase();

protected:
    TextureRef skyBoxTexture_;

    // PSOのBlendModeのマップ
    std::unordered_map<RenderMode, std::unordered_map<BlendMode, uint32_t>> psoIDs_;
    std::unordered_map<RenderMode, uint32_t> rootSigIDs_;

    /// <summary>
    /// psoIDs_ / rootSigIDs_ を初回のみ構築する
    /// </summary>
    void EnsurePSOsInitialized();

    /// <summary>
    /// 収集したDrawItemの情報から描画します
    /// </summary>
    /// <param name="context">描画用コマンドリストのラッパークラスの参照</param>
    /// <param name="resourceRegistry"></param>
    /// <param name="items"></param>
    void RenderItems(GraphicsContext& context, const RenderGraphRegistry& resourceRegistry,
        const std::vector<RenderContext::MeshDrawItem>& items, const Component::CameraComponent& camera, ECS::Registry& registry);
};

/// <summary>
/// 不透明メッシュとアウトラインを描画するレンダーパス。
/// 半透明メッシュはTransparentMeshPass(DepthOfField後)が担当する。
/// </summary>
class MeshPass :
    public MeshPassBase {
public:
    /// <summary>
    /// メッシュ描画を実行する
    /// </summary>
    /// <param name="gfx">描画用コマンドリストのラッパークラスの参照</param>
    /// <param name="registry">ECSレジストリ</param>
    virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;

private:
    std::string outlinePSOName_;
    uint32_t outlinePSOID_ = 0;

    /// <summary>
    /// GraphicsContext を使用してアウトラインを描画する関数。
    /// </summary>
    /// <param name="gfx">描画操作に使用する GraphicsContext への参照。</param>
    void RenderOutline(GraphicsContext& gfx, const std::vector<RenderContext::MeshDrawItem>& items,
        const Component::CameraComponent& camera, ECS::Registry& registry);
};
}
}
