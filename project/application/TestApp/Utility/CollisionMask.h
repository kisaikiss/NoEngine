#pragma once
#include <cstdint>

namespace TestApp {

	/// <summary>
	/// CollisionType
	/// このコライダーの種類
	/// </summary>
	enum class CollisionType : uint32_t {
		None = 0,				//無効（何とも衝突しない）
		Player = 1 << 0,		//プレイヤー本体
		Enemy = 1 << 1,			//敵本体
		PlayerBullet = 1 << 2,	//プレイヤーの弾
		EnemyBullet = 1 << 3,	//敵の弾
		Block = 1 << 4,			//地形・壁・障害物
		Item = 1 << 5,			//アイテム
		//拡張
		All = ~0u,				//全ビットON（デバッグ）
	};

	/// <summary>
	/// CollisionMask プリセット
	/// 衝突判定する対象の種類+
	/// </summary>
	namespace CollisionMask {

		/// プレイヤーが当たり判定を取る相手
		/// 敵本体 / 敵の弾 / 地形
		constexpr CollisionType Player =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Enemy) |
				static_cast<uint32_t>(CollisionType::EnemyBullet) |
				static_cast<uint32_t>(CollisionType::Block)
				);

		/// 敵が当たり判定を取る相手
		/// プレイヤー本体 / プレイヤーの弾 / 地形
		constexpr CollisionType Enemy =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Player) |
				static_cast<uint32_t>(CollisionType::PlayerBullet) |
				static_cast<uint32_t>(CollisionType::Block)
				);

		/// プレイヤーの弾が当たり判定を取る相手
		/// 敵本体 / 地形
		constexpr CollisionType PlayerBullet =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Enemy) |
				static_cast<uint32_t>(CollisionType::Block)
				);

		/// 敵の弾が当たり判定を取る相手
		/// プレイヤー本体 / 地形
		constexpr CollisionType EnemyBullet =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Player) |
				static_cast<uint32_t>(CollisionType::Block)
				);

		/// 地形が当たり判定を取る相手
		/// プレイヤー / 敵 / 各種弾
		constexpr CollisionType Block =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Player) |
				static_cast<uint32_t>(CollisionType::Enemy) |
				static_cast<uint32_t>(CollisionType::PlayerBullet) |
				static_cast<uint32_t>(CollisionType::EnemyBullet)
				);

		/// アイテムが当たり判定を取る相手
		/// プレイヤーのみ
		constexpr CollisionType Item =
			static_cast<CollisionType>(
				static_cast<uint32_t>(CollisionType::Player)
				);

		/// デバッグ・特殊用途: 全レイヤーと衝突
		constexpr CollisionType All = CollisionType::All;

		/// 無効: 何とも衝突しない
		constexpr CollisionType None = CollisionType::None;

	}




	// ビット演算子オーバーロード
	constexpr CollisionType operator|(CollisionType lhs, CollisionType rhs) {
		return static_cast<CollisionType>(
			static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
			);
	}

	constexpr CollisionType operator&(CollisionType lhs, CollisionType rhs) {
		return static_cast<CollisionType>(
			static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)
			);
	}

	constexpr CollisionType operator~(CollisionType val) {
		return static_cast<CollisionType>(~static_cast<uint32_t>(val));
	}

	constexpr CollisionType& operator|=(CollisionType& lhs, CollisionType rhs) {
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr CollisionType& operator&=(CollisionType& lhs, CollisionType rhs) {
		lhs = lhs & rhs;
		return lhs;
	}

	// CollisionType 同士の比較
	constexpr bool operator==(CollisionType lhs, CollisionType rhs) {
		return static_cast<uint32_t>(lhs) == static_cast<uint32_t>(rhs);
	}
	constexpr bool operator!=(CollisionType lhs, CollisionType rhs) {
		return static_cast<uint32_t>(lhs) != static_cast<uint32_t>(rhs);
	}


	// uint32_t との比較オーバーロード
	constexpr bool operator==(CollisionType lhs, uint32_t rhs) {
		return static_cast<uint32_t>(lhs) == rhs;
	}
	constexpr bool operator!=(CollisionType lhs, uint32_t rhs) {
		return static_cast<uint32_t>(lhs) != rhs;
	}
	constexpr bool operator==(uint32_t lhs, CollisionType rhs) {
		return lhs == static_cast<uint32_t>(rhs);
	}
	constexpr bool operator!=(uint32_t lhs, CollisionType rhs) {
		return lhs != static_cast<uint32_t>(rhs);
	}
}
