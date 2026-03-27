#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"
#include <map>

/// <summary>
/// 敵種類ごとのプリセット設定構造体
/// </summary>
struct EnemyTypePreset {
	float modelScale = 0.7f;
	No::Vector3 baseColliderBox = { 1.0f, 1.0f, 1.0f };
	float shootInterval = 1.0f;					// 敵がプレイヤーに向かって撃つ間隔。これが小さいほど、敵は頻繁に撃つ。
	float bulletSpeed = 10.0f;					// 敵が撃つ弾の速度
	int bulletDamage = 1;						// 敵が撃つ弾のダメージ
	float bulletLifetime = 4.0f;				// 敵が撃った弾が消えるまでの時間
	float targetDepthFromCamera = 1.0f;			// 敵がプレイヤーを撃つときの、カメラからの深さ。これが小さいほど、敵はカメラに近い位置を狙う。
	float shootDistanceMax = 25.0f;				// 敵がプレイヤーを撃つ最大距離。
	float despawnBehindDistance = 12.0f;		// カメラより後ろにこの距離以上離れたらスポーンしている敵を消す
	int minHp = 1;
};

using EnemyTypePresetMap = std::map<RailEnemyType, EnemyTypePreset>;

std::string MakeEnemyTypePresetFilePath();
bool LoadEnemyTypePresetMap(EnemyTypePresetMap& outPresets);
bool SaveEnemyTypePresetMap(const EnemyTypePresetMap& presets);
EnemyTypePreset GetEnemyTypePresetOrDefault(const EnemyTypePresetMap& presets, RailEnemyType type);
