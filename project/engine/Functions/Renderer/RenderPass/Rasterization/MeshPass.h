#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Render {

/// <summary>
/// MeshPass / TransparentMeshPass が共有するメッシュ収集・ソート・描画ロジックの基底クラス。
/// DrawItemの収集(Collect)は不透明・半透明の両方を毎回まとめて行い、
/// 各派生クラスは自分が使うリストだけをRenderItemsに渡す。
/// </summary>
class MeshPassBase :
    public RenderPass {
public:
    MeshPassBase();

protected:
    /// <summary>
    /// 描画に必要な変数
    /// </summary>
    struct DrawItem {
        Asset::MeshHandle meshHandle;
        std::vector<Asset::MaterialHandle> materialHandles;
        Component::MaterialComponent* material;
        Component::TransformComponent* transform;
        Transform* animationLocal;
        float distanceToCamera;
    };

    std::vector<DrawItem> opaqueItems_;
    std::vector<DrawItem> transparentItems_;
    Component::CameraComponent* camera_;
    TextureRef skyBoxTexture_;

    // PSOのBlendModeのマップ
    std::unordered_map<RenderMode, std::unordered_map<BlendMode, uint32_t>> psoIDs_;
    std::unordered_map<RenderMode, uint32_t> rootSigIDs_;

    /// <summary>
    /// psoIDs_ / rootSigIDs_ を初回のみ構築する
    /// </summary>
    void EnsurePSOsInitialized();

    /// <summary>
    /// DrawItemを収集します(不透明・半透明の両方をopaqueItems_ / transparentItems_へ振り分け)
    /// </summary>
    /// <param name="registry">ECSレジストリ</param>
    virtual void Collect(ECS::Registry& registry);

    /// <summary>
    /// DrawItemの中身をソートします。
    /// </summary>
    void Sort();

    /// <summary>
    /// 収集したDrawItemの情報から描画します
    /// </summary>
    /// <param name="context">描画用コマンドリストのラッパークラスの参照</param>
    /// <param name="resourceRegistry"></param>
    /// <param name="items"></param>
    void RenderItems(GraphicsContext& context, const RenderGraphRegistry& resourceRegistry, const std::vector<DrawItem>& items, ECS::Registry& registry);
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
    void RenderOutline(GraphicsContext& gfx, ECS::Registry& registry);
};
}
}