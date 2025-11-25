#pragma once
namespace NoEngine {
enum class  LinearAllocatorType {
    kInvalidAllocator = -1,

    kGpuExclusive = 0,		// デフォルト GPU 書き込み可能 (UAV 経由)
    kCpuWritable = 1,		// アップロード CPU 書き込み可能 (ただし書き込みは結合)

    kNumAllocatorTypes
};
}