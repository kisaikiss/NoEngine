#pragma once
namespace NoEngine {
/// <summary>
/// コピー不可の構造体。コピー不可のクラスをつくりたいときにはこれを継承します。
/// </summary>
struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
}