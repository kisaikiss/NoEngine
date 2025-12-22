#pragma once
namespace NoEngine {
namespace Math {
	//  __forceinlineによってinlne展開を強制し、cpu処理速度の高速化を試みます。
	
	/// <summary>
	/// マスクを使って高速にalignUpします。
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="value">切り上げたい値</param>
	/// <param name="mask">下位ビットを1にしたビットマスク</param>
	/// <returns>切り上げ結果</returns>
	template <typename T> 
	__forceinline T AlignUpWithMask(T value, size_t mask) {
		return (T)(((size_t)value + mask) & ~mask);
	}

	/// <summary>
	/// マスクを使って高速にalignDownします。
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="value">切り下げたい値</param>
	/// <param name="mask">下位ビットを1にしたビットマスク</param>
	/// <returns>切り下げ結果</returns>
	template <typename T>
	__forceinline T AlignDownWithMask(T value, size_t mask) {
		return  (T)((size_t)value & ~mask);
	}

	/// <summary>
	/// 値をalignmentバイト領界に切り上げます。
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="value">切り上げたい値</param>
	/// <param name="alignment">2べき乗のalignment</param>
	/// <returns>切り上げ結果</returns>
	template <typename T>
	__forceinline T AlignUp(T value, size_t alignment) {
		return AlignUpWithMask(value, alignment - 1);
	}

	/// <summary>
	/// 値をalignmentバイト領界に切り下げます。
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="value">切り下げたい値</param>
	/// <param name="alignment">2べき乗のalignment</param>
	/// <returns>切り下げ結果</returns>
	template <typename T>
	__forceinline T AlignDown(T value, size_t alignment) {
		return AlignDownWithMask(value, alignment - 1);
	}

	template <typename T>
	__forceinline bool IsAligned(T value, size_t alignment) {
		return 0 == ((size_t)value & (alignment - 1));
	}

	template <typename T>
	__forceinline T DivideByMultiple(T value, size_t alignment) {
		return (T)((value + alignment - 1) / alignment);
	}
}
}