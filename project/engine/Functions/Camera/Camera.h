#pragma once
#include "CameraBase.h"
namespace NoEngine {
class Camera :
    public CameraBase {
public:
    Camera();

    void SetFOV(float fovY) { fovY_ = fovY; UpdateProjectionMatrix(); }
    void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; UpdateProjectionMatrix(); }
private:
    void UpdateProjectionMatrix();

    float fovY_;
    float aspectRatio_;
    float farClip_;
    float nearClip_;
};
}

