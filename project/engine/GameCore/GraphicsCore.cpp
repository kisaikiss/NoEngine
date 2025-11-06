#include "GraphicsCore.h"

namespace NoEngine {
namespace Graphics {
using namespace std;
void GraphicsCore::Initialize() {
	graphicsInfrastructures_ = make_unique<GraphicsInfrastructures>();
	graphicsDevice_ = make_unique<GraphicsDevice>(graphicsInfrastructures_->GetDXGIAdapter());
}
}
}