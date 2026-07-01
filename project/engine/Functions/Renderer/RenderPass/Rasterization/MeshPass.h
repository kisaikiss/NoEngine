#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Render {
/// <summary>
/// メッシュを描画するレンダーパス
/// </summary>
class MeshPass :
    public RenderPass {
public:
    /// <summary>
    /// メッシュパスのコンストラクタ
    /// </summary>
    MeshPass();

    /// <summary>
    /// メッシュ描画を実行する
    /// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラスの参照</param>
	/// <param name="registry">ECSレジストリ</param>
    virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
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
        uint32_t psoId;
        uint32_t rootSigId;
        std::string psoName;
        float distanceToCamera;
    };

    std::vector<DrawItem> items_;
    Component::CameraComponent* camera_;

    std::string outlinePSOName_;
    uint32_t outlinePSOID_;
    TextureRef skyBoxTexture_;

    /// <summary>
    /// DrawItemを収集します
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
    /// <param name="gfx">描画用コマンドリストのラッパークラスの参照</param>
    void Render(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry);

    /// <summary>
    /// GraphicsContext を使用してアウトラインを描画する関数。
    /// </summary>
    /// <param name="gfx">描画操作に使用する GraphicsContext への参照。</param>
    void RenderOutline(GraphicsContext& gfx);
};
}
}
