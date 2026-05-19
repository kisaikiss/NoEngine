#pragma once
#include "../RenderContext.h"
namespace NoEngine {

class PostEffectInitialzer {
    friend class RenderInitializer;
    static void Initialize(RenderContext& ctx);
    static void CreatePSO(RenderContext& ctx);
};

}