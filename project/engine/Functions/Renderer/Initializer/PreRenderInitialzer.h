#pragma once
#include "../RenderContext.h"
namespace NoEngine {

class PreRenderInitialzer {
    friend class RenderInitializer;
    static void Initialize(RenderContext& ctx);
    static void CreatePSO(RenderContext& ctx);
};

}