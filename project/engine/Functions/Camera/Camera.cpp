#include "Camera.h"

#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
Camera::Camera() : 
fovY_(0.45f),
nearClip_(0.1f),
farClip_(1000.f){
	// ToDo : 現在はメインウィンドウからのみしかaspectRatioを取得できませんが、自由にあらゆるウィンドウのaspectRatioを取得できるようにすべきです。
	aspectRatio_ = GraphicsCore::gWindowManager.GetMainWindow()->GetAspectRatio();
	UpdateProjectionMatrix();
	Update();
	
}

void Camera::UpdateProjectionMatrix() {
	projectionMatrix_ = MathCalculations::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
}

}