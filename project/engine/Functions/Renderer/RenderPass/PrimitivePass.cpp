#include "PrimitivePass.h"
#include "../Primitive.h"

namespace NoEngine
{
    namespace Render
    {
        PrimitivePass::PrimitivePass()
        {
            Primitive::Initialize();
        }
        void PrimitivePass::Execute(GraphicsContext& gfx, ECS::Registry& registry)
        {
            (void)registry;
            Primitive::Render(gfx,GetCamera()->GetViewProjMatrix());
        }
    }
}