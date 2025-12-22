#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
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

class MeshRenderer {
public:
	/// <summary>
	/// レンダラーのイニシャライズ関数。シェーダーコンパイルやPSO生成などを行います。
	/// </summary>
	static void Initialize(void);

    static void Shutdown(void);

    static void Render(GraphicsContext& context);
private:
};
}

