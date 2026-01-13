#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Runtime/DescriptorHeap/DescriptorHeap.h"
namespace NoEngine {


enum class RootBindings {
    kMeshConstants,
    kMaterialConstants,
    kMaterialSRVs,
    kMaterialSamplers,
    kCommonSRVs,
    kCommonCBV,
    kSkinMatrices,

    kNumRootBindings
};

namespace MeshRenderer {
	/// <summary>
	/// レンダラーのイニシャライズ関数。シェーダーコンパイルやPSO生成などを行います。
	/// </summary>
	void Initialize(void);

    void Shutdown(void);

    void Render(GraphicsContext& context);

    extern DescriptorHeap gTextureHeap;
};
}

