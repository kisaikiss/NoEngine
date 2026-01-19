#pragma once
#include "RenderPass.h"

namespace NoEngine
{
    namespace Render
    {
        class PrimitivePass :
            public RenderPass
        {
        public:
            PrimitivePass();
            ~PrimitivePass() = default;
            void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
        };
    }
}