#pragma once
namespace NoEngine {
struct RaytracingGeometry {
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
};

struct RaytracingMesh {
    // BLAS
    Microsoft::WRL::ComPtr<ID3D12Resource> blas;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
    std::vector<RaytracingGeometry> geometries;
};
}