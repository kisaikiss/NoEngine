#pragma once
#define NOMINMAX

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#include <d3d12.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <wrl.h>

#include <externals/DirectXTex/d3dx12.h>

#include <memory>
#include <vector>
#include <list>
#include <cassert>
#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>
#include <optional>
#include <functional>
#include <format>
#include <mutex>
#include <queue>
#include <array>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <span>
#include <any>
#include <set>