#pragma once
#include "../Math/Common.h"

// pragma intrinsicで強制的に_mm_crc32_u32と_mm_crc32_u64をintrinsic化します(cpu命令に直接展開するようにする)。
#pragma intrinsic(_mm_crc32_u32)
#pragma intrinsic(_mm_crc32_u64)

namespace NoEngine {
namespace Utility {
/// <summary>
/// 指定したメモリ範囲をハッシュ化します。
/// </summary>
/// <param name="begin">メモリ範囲の初め</param>
/// <param name="end">メモリ範囲の終わり</param>
/// <param name="hash">ハッシュの初期値</param>
/// <returns>ハッシュ値</returns>
inline size_t HashRange(const uint32_t* const begin, const uint32_t* const end, size_t hash) {
    // 8バイト領界にアラインアップ
    const uint64_t* iter64 = (const uint64_t*)Math::AlignUp(begin, 8);
    // 8バイト領界にアラインダウン
    const uint64_t* const end64 = (const uint64_t* const)Math::AlignDown(end, 8);

    // _mm_crc32_u32()などはCPUのCRC命令が実行される関数です。
    // 定義は_mm_crc32_u32(uint32_t crc, uint32_t data)となっており、
    // crcとdataを合成した新しいcrc値を返します。

    // 64ビットに揃えていない場合は、単一のu32から始めます。
    if ((uint32_t*)iter64 > begin) {
        hash = _mm_crc32_u32(static_cast<uint32_t>(hash), *begin);
    }

    // 連続するu64値を反復処理します。
    while (iter64 < end64) {
        hash = _mm_crc32_u64(static_cast<uint64_t>(hash), *iter64++);
    }

    // 32ビットの余りがある場合はそれを累算します。
    if ((uint32_t*)iter64 < end) {
        hash = _mm_crc32_u32(static_cast<uint32_t>(hash), *(uint32_t*)iter64);
    }

    return hash;
}

/// <summary>
/// 任意の構造体をハッシュ化します。
/// </summary>
/// <typeparam name="T">任意の構造体</typeparam>
/// <param name="stateDesc">任意の構造体のポインタ</param>
/// <param name="count">配列数(通常は1)</param>
/// <param name="hash">ハッシュの初期値</param>
/// <returns>ハッシュ値</returns>
template <typename T> 
inline size_t HashState(const T* stateDesc, size_t count = 1, size_t hash = 2166136261U) {
    // 構造体サイズが4バイト領界で割り切れて、かつアライメントが4バイト以上なら通します。そうでないならコンパイルエラーにします。
    static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
    return HashRange((uint32_t*)stateDesc, (uint32_t*)(stateDesc + count), hash);
}
}
}