#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Collision/Component/ColliderConfig.h"
#include <map>

/// <summary>
/// 敵種類ごとの設定構造体。
/// EnemyTypePreset から移行・リファクタリングした新しい名前。
/// </summary>
struct EnemyConfig {
    float modelScale = 0.7f;
    float shootInterval = 1.0f;                 // 敵がプレイヤーに向かって撃つ間隔。これが小さいほど、敵は頻繁に撃つ。
    float bulletSpeed = 10.0f;                  // 敵が撃つ弾の速度
    int bulletDamage = 1;                       // 敵が撃つ弾のダメージ
    float bulletLifetime = 4.0f;               // 敵が撃った弾が消えるまでの時間
    float targetDepthFromCamera = 1.0f;        // 敵がプレイヤーを撃つときの、カメラからの深さ。これが小さいほど、敵はカメラに近い位置を狙う。
    float shootDistanceMax = 25.0f;            // 敵がプレイヤーを撃つ最大距離。
    float despawnBehindDistance = 12.0f;       // カメラより後ろにこの距離以上離れたらスポーンしている敵を消す
    int minHp = 1;

    float bulletModelScale = 0.25f;  // 弾モデルのスケール（旧ハードコード値 0.25f を初期値とする）

    // コライダー設定（旧 baseColliderBox は enemyCollider.boxSizeMultiplier へ移行）
    ColliderConfig enemyCollider;    // 敵本体のコライダー設定
    ColliderConfig bulletCollider;   // 敵の弾のコライダー設定
};

/// <summary>
/// 敵種類ごとの設定マップ。旧 EnemyTypePresetMap に対応。
/// </summary>
using EnemyConfigMap = std::map<RailEnemyType, EnemyConfig>;
