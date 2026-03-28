# NoEngine / CommentBout 実装計画書

> 作成日: 2026-03-28
> ブランチ: `Collision_Update`
> 目的: 全未実装機能の確実な実装のための手順書（自分用チェックリスト）

---

## AI 行動規約

**Claude はこの作業において以下を厳守する：**

- `git push` は一切行わない（ローカル作業のみ）
- `git commit` はユーザーが明示的に指示した場合のみ行う
- リモートリポジトリへの操作（push / force-push / PR作成 など）は禁止
- 外部サービスへのデータ送信は行わない
- 破壊的操作（ファイル大量削除・ブランチ削除など）は事前確認する

---

## 目次

1. [全体方針](#全体方針)
2. [実装順序一覧](#実装順序一覧)
3. [Phase 1 — C: 命名統一 + コライダーオフセット](#phase-1--c-命名統一--コライダーオフセット)
4. [Phase 2 — D: 吹き出しリネーム + サイズ分け](#phase-2--d-吹き出しリネーム--サイズ分け)
5. [Phase 3 — E: StageData 構造整備](#phase-3--e-stagedata-構造整備)
6. [Phase 4 — A: コントローラー対応](#phase-4--a-コントローラー対応)
7. [Phase 5 — G: エディタ管理クラス](#phase-5--g-エディタ管理クラス)
8. [Phase 5.5 — G2: エディタ大幅リファクタリング](#phase-55--g2-エディタ大幅リファクタリング)
9. [Phase 6 — H前半: ボス構造変更](#phase-6--h前半-ボス構造変更)
10. [Phase 7 — H後半: クリア/オーバー演出](#phase-7--h後半-クリアオーバー演出)
11. [Phase 8 — I: 音実装](#phase-8--i-音実装)
12. [Phase 9 — F: フォルダ精査・整理](#phase-9--f-フォルダ精査整理)

---

## 全体方針

### アーキテクチャ原則（変えない）
- ECS (Registry / Component / System) の構造を維持
- データ (Config) とロジック (System) とIO (DataIO) を分離する
- ImGui エディタはゲームロジックに混入させない

### 命名規則（Phase 1 以降統一）

| 種別 | 命名パターン | 例 |
|------|------------|-----|
| ゲームデータ構造体 | `XxxConfig` | `PlayerConfig`, `EnemyConfig` |
| JSON 読み書きクラス | `XxxDataIO` | `PlayerDataIO`, `EnemyDataIO` |
| ECS コンポーネント | `XxxComponent` | `HealthComponent`, `BossComponent` |
| ECS タグ | `CBXxxTag` | `CBPlayerTag`, `CBBossTag` |
| ECS システム | `XxxSystem` | `DamageApplySystem` |
| エディタクラス | `XxxEditor` | `RailCameraEditor`, `FieldObjectEditor` |

### スプライト素材対応表（H演出用）

| 用途 | ファイル | 備考 |
|------|---------|------|
| ボス撃破エフェクト | `BOOM.png` (kAttackOrbEffectMedium 流用) | 既存 |
| 暗幕フェード | `resources/engine/white1x1.png` | 既存。黒カラーで着色 |
| クリアロゴ（仮） | `resources/game/td_3105/Sprite/TitleRogo.png` | 後で差し替え |
| オーバーロゴ（仮） | `resources/game/td_3105/Sprite/TitleRogo.png` | 後で差し替え |
| リスタート/タイトルボタン | ポーズと共用 (`kRestart`, `kPauseToTitle`) | 既存 |

---

## 実装順序一覧

```
Phase 1    C    命名統一 + コライダーオフセット（基盤）          ✅ 完了
Phase 2    D    吹き出しリネーム + サイズ分け                  ✅ 完了
Phase 3    E    StageData 構造整備                            ✅ 完了
Phase 4    A    コントローラー対応                             ✅ 完了
Phase 5    G    エディタ管理クラス                             ✅ 完了
Phase 5.5  G2   エディタ大幅リファクタリング                    ✅ 完了
Phase 6    H前半 ボス構造変更                              ✅ 完了
Phase 7    H後半 クリア/オーバー演出                          ✅ 完了
Phase 8    I    音実装
Phase 9    F    フォルダ精査・整理
```

---

## Phase 1 — C: 命名統一 + コライダーオフセット

> **目的:** データ構造の命名を統一し、コライダーにオフセット機能を追加する。
> **影響ファイル (既存):**
> - `Component/PlayerConfigComponent.h`
> - `Data/EnemyTypePresetIO.h/.cpp`
> - `Data/RailDataIO.cpp`
> - `Scene/GameScene.cpp`
> - `System/EnemySpawnSystem.cpp`
> - `System/DamageApplySystem.cpp`
> - `System/PlayerInfoDebugSystem.cpp`
> - `Editor/GameEventEditor.cpp`
> - `Collision/Component/Collider3DComponent.h`
> - `Collision/Component/Collider2DComponent.h`
> - `Collision/System/CollisionSystem.cpp` (UpdateCollider3D / UpdateCollider2D)

---

### 1-1. ColliderConfig 汎用データ構造を作成

**新規ファイル:** `Collision/Component/ColliderConfig.h`

```cpp
struct ColliderConfig {
    No::Vector3 localOffset3D{ 0.f, 0.f, 0.f };  // 3D コライダーのローカルオフセット
    No::Vector2 localOffset2D{ 0.f, 0.f };        // 2D コライダーのローカルオフセット
    float radiusMultiplier = 1.0f;                 // 球半径の倍率
    No::Vector3 boxSizeMultiplier{ 1.f, 1.f, 1.f }; // ボックスサイズの倍率
    No::Vector2 sizeMultiplier2D{ 1.f, 1.f };     // 2D サイズの倍率
};
// JSON シリアライズ関数 (to_json / from_json) もここに
```

- [x] `ColliderConfig.h` を新規作成
- [x] `to_json` / `from_json` 実装
- [x] `vcxproj` / `vcxproj.filters` に追加

---

### 1-2. Collider3DComponent にオフセットを追加

**対象:** `Collision/Component/Collider3DComponent.h`

追加フィールド:
```cpp
No::Vector3 localOffset{ 0.f, 0.f, 0.f };  // ← 追加
```

`UpdateCollider3D()` の `worldPosition` 計算を修正:
```cpp
// 変更前: worldPosition = transform->GetWorldPosition()
// 変更後: worldPosition = transform->GetWorldPosition() + rotatedOffset
// (TransformのrotationでlocalOffsetを回転してから加算)
```

- [x] `Collider3DComponent.h` に `localOffset` フィールド追加
- [x] `CollisionSystem.cpp` の `UpdateCollider3D()` でオフセット適用
- [x] オフセット適用: `No::Matrix4x4 rot = transform.MakeRotationMatrix(); worldPosition = pos + rot.TransformNormal(localOffset);`

---

### 1-3. Collider2DComponent にオフセットを追加

**対象:** `Collision/Component/Collider2DComponent.h`

追加フィールド:
```cpp
No::Vector2 localOffset{ 0.f, 0.f };  // ← 追加
```

`UpdateCollider2D()` の `screenPosition` 計算を修正:
```cpp
// 変更後: screenPosition = transform2D->translate + localOffset
```

- [x] `Collider2DComponent.h` に `localOffset` フィールド追加
- [x] `CollisionSystem.cpp` の `UpdateCollider2D()` でオフセット適用

---

### 1-4. PlayerConfigComponent → PlayerConfig にリネーム

**旧:** `Component/PlayerConfigComponent.h` (struct + inline save/load)
**新:** `Data/PlayerConfig.h` (データ構造のみ) + `Data/PlayerDataIO.h/.cpp` (IO分離)

**プレイヤーの当たり判定構造（前提知識）:**
プレイヤーには Collider3D は存在しない。2種類の判定がある：
- `Collider2DComponent` → 画面上のスプライト当たり判定（サイズ・オフセット設定対象）
- `PlayerHitboxComponent` (カメラゲート) → 3D空間でのダメージ受け判定。
  パラメータ (`cameraGateHalfWidth` 等) は現在 `PlayerConfigComponent` に重複して存在する。

**変更内容:**
- `struct PlayerConfigComponent` → `struct PlayerConfig`
- `ColliderConfig playerCollider2D` フィールドを追加（Collider2D のオフセット・サイズ）
- `ColliderConfig attackCollider` フィールドを追加（プレイヤー攻撃の Collider2D）
- カメラゲートパラメータ (`useCameraGateForPlayerHit`, `cameraGateNear`, `cameraGateDepth`,
  `cameraGateHalfWidth`, `cameraGateHalfHeight`) はそのまま `PlayerConfig` に残す
- `PlayerHitboxComponent` の値は `PlayerConfig` から適用する形に整理
- `LoadPlayerConfig()` / `SavePlayerConfig()` を `PlayerDataIO` に移動
- JSONパスを新構造に変更: `"resources/game/td_3105/Data/Config/PlayerConfig.json"`

**影響ファイルの変更 (grep で確認して全修正):**

| ファイル | 変更内容 |
|---------|---------|
| `Component/PlayerConfigComponent.h` | ファイルごと `Data/PlayerConfig.h` に移動 |
| `Scene/GameScene.cpp` | include パス + 型名変更 |
| `System/DamageApplySystem.cpp` | 型名 `PlayerConfigComponent` → `PlayerConfig` |
| `System/PlayerInfoDebugSystem.cpp` | 型名変更 + ImGui でコライダー設定表示追加 |
| `vcxproj` / `vcxproj.filters` | ファイルパス更新 |

- [x] `Data/PlayerConfig.h` 新規作成 (ColliderConfig フィールド含む)
- [x] `Data/PlayerDataIO.h/.cpp` 新規作成
- [x] `PlayerDataIO::Load()` / `::Save()` 実装 (nlohmann::json)
- [x] `PlayerConfig` の `ColliderConfig` を JSON に含める
- [x] 全参照ファイルの型名・include を更新
- [x] `vcxproj` / `vcxproj.filters` 更新
- [x] 旧 `PlayerConfigComponent.h` を削除 (互換ヘッダーとして残存)

---

### 1-5. EnemyTypePreset → EnemyConfig にリネーム

**旧:** `Data/EnemyTypePresetIO.h/.cpp`
**新:** `Data/EnemyConfig.h` + `Data/EnemyDataIO.h/.cpp`

**変更内容:**
- `struct EnemyTypePreset` → `struct EnemyConfig`
- `EnemyTypePresetMap` → `EnemyConfigMap` (= `std::unordered_map<std::string, EnemyConfig>`)
- `ColliderConfig enemyCollider` フィールドを追加 (旧 `baseColliderBox` を移行)
- `ColliderConfig bulletCollider` フィールドを追加
- `EnemyTypePresetIO` → `EnemyDataIO`
- JSONパス: `"resources/game/td_3105/Data/Config/EnemyConfig.json"`

**影響ファイル:**

| ファイル | 変更内容 |
|---------|---------|
| `Data/EnemyTypePresetIO.h/.cpp` | リネーム + 型名変更 |
| `System/EnemySpawnSystem.cpp` | 型名変更 + bulletCollider を Collider3DComponent に適用 |
| `Editor/GameEventEditor.cpp` | 型名変更 |
| `Data/RailDataIO.cpp` | 型名変更 |
| `vcxproj` / `vcxproj.filters` | ファイルパス更新 |

- [x] `Data/EnemyConfig.h` 新規作成
- [x] `Data/EnemyDataIO.h/.cpp` 新規作成 (旧 EnemyTypePresetIO の内容を移行)
- [x] `EnemyConfig` に `ColliderConfig enemyCollider` / `bulletCollider` 追加
- [x] `EnemySpawnSystem.cpp` の敵生成時に `enemyCollider.localOffset3D` を `Collider3DComponent.localOffset` に設定
- [x] `EnemySpawnSystem.cpp` の弾生成時に `bulletCollider` を適用
- [x] 全参照ファイルの型名・include を更新
- [x] 旧 `EnemyTypePresetIO.h/.cpp` を削除 (互換のため残存)

---

### 1-6. ImGui でコライダー設定を編集 + JSON 保存

**PlayerInfoDebugSystem** に PlayerConfig のコライダー設定 ImGui を追加:
```
[Player Collider Config]
  localOffset3D:  X[___] Y[___] Z[___]
  attackOffset2D: X[___] Y[___]
  radiusMultiplier: [___]
  attackSizeMultiplier: X[___] Y[___]
[Save] → PlayerDataIO::Save()
```

**GameEventEditor** に EnemyConfig のコライダー設定 ImGui を追加 (タイプ別):
```
[Enemy Collider Config: {TypeKey}]
  enemyCollider.localOffset3D: X Y Z
  bulletCollider.radiusMultiplier: [___]
[Save] → EnemyDataIO::Save()
```

- [x] `PlayerInfoDebugSystem` に `ColliderConfig` ImGui 追加 (Collider2D + カメラゲート両方)
- [x] `GameEventEditor` に `EnemyConfig.bulletCollider` ImGui 追加
- [x] 保存ボタンで `PlayerDataIO::Save()` / `EnemyDataIO::Save()` 呼び出し

---

### Phase 1 完了確認

- [x] ビルドエラーなし
- [x] 実行時エラーなし (コライダー位置がずれていないか目視確認)
- [x] PlayerConfig.json が新パスで読み書きできる
- [x] EnemyConfig.json が新パスで読み書きできる
- [x] コライダーのオフセット値を変更すると当たり判定がずれる

---

## Phase 2 — D: 吹き出しリネーム + サイズ分け

> **目的:** オーブを「吹き出し」にリネームし、カメラ距離に応じて大中小サイズに分ける。
> **前提:** Phase 1 完了 (ColliderConfig が使える状態)
> **影響ファイル:**
> - `Component/EnemyRewardOrbComponent.h`
> - `System/EnemyRewardToBossSystem.h/.cpp`
> - `System/EnemyVisualSystem.cpp` (SpawnRewardOrbFromEnemy)
> - `Component/GameResourceComponent.h`
> - 新規: `Data/SpeechBubbleConfig.h` + `Data/SpeechBubbleDataIO.h/.cpp`

---

### 2-1. リネーム: Orb → SpeechBubble

| 旧名 | 新名 |
|-----|-----|
| `EnemyRewardOrbComponent` | `SpeechBubbleComponent` |
| `CBEnemyRewardOrbTag` | `CBSpeechBubbleTag` |
| `EnemyRewardToBossSystem` | `SpeechBubbleToBossSystem` |
| `SpawnRewardOrbFromEnemy()` | `SpawnSpeechBubbleFromEnemy()` |
| `kAttackOrbEffectSmall` | `kSpeechBubbleSmall` |
| `kAttackOrbEffectMedium` | `kSpeechBubbleMedium` |
| `kAttackOrbEffectBig` | `kSpeechBubbleLarge` |
| `kRewardOrbSprite` | `kSpeechBubbleSprite` (使用停止 or 削除) |

- [x] `SpeechBubbleComponent.h` 新規作成 (EnemyRewardOrbComponent は互換ヘッダーとして残存)
- [x] struct 名・タグ名を変更 (CBSpeechBubbleTag, CBSpeechBubbleConfigTag 追加)
- [x] `SpeechBubbleToBossSystem.h/.cpp` 新規作成
- [x] `GameResourceComponent.h` のキー定数をリネーム (kSpeechBubbleLarge/Medium/Small)
- [x] `EnemyVisualSystem.cpp` の関数名・タグ参照を更新
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 2-2. SpeechBubbleConfig データ構造を作成

**新規ファイル:** `Data/SpeechBubbleConfig.h`

```cpp
struct SpeechBubbleSizeConfig {
    No::Vector2 spriteSize{ 80.f, 80.f };
    int attackPower = 1;
    float duration = 0.9f;
    // 将来的に ColliderConfig も追加可能
};

struct SpeechBubbleConfig {
    // 距離閾値 (カメラからの3D距離)
    float largeMaxDistance  = 8.0f;   // 0 〜 largeMax → Large
    float mediumMaxDistance = 16.0f;  // largeMax 〜 mediumMax → Medium
                                       // mediumMax 〜 → Small

    SpeechBubbleSizeConfig large;   // POW.png 使用
    SpeechBubbleSizeConfig medium;  // BOOM.png 使用
    SpeechBubbleSizeConfig small;   // OH.png 使用
};
// to_json / from_json
```

- [x] `Data/SpeechBubbleConfig.h` 新規作成
- [x] `Data/SpeechBubbleDataIO.h/.cpp` 新規作成
- [x] JSON パス: `"resources/game/td_3105/Data/Config/SpeechBubbleConfig.json"`
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 2-3. SpeechBubbleComponent を更新

```cpp
struct SpeechBubbleComponent {
    No::Vector2 start, control, end;
    float duration = 0.9f;
    float elapsed = 0.0f;
    int attackPower = 1;
    int sizeCategory = 1;  // 0=Large, 1=Medium, 2=Small  ← 追加
};
```

- [x] `SpeechBubbleComponent.h` に `sizeCategory` フィールド追加

---

### 2-4. SpawnSpeechBubbleFromEnemy() の距離判定実装

**対象:** `EnemyVisualSystem.cpp`

```
1. RailCameraComponent を持つエンティティの Transform3D を取得
2. 死亡した敵の worldPosition を取得 (Collider3D の worldPosition から)
3. カメラ位置との3D距離を計算: distance = length(enemyWorldPos - cameraWorldPos)
4. 距離に応じてサイズ決定:
     if (distance <= config.largeMaxDistance)  → Large (POW.png)
     else if (distance <= config.mediumMaxDistance) → Medium (BOOM.png)
     else → Small (OH.png)
5. 対応する SpeechBubbleSizeConfig から spriteSize, attackPower, duration を使用
6. 対応するスプライトキー (kSpeechBubbleLarge/Medium/Small) でテクスチャ設定
```

- [x] `SpawnSpeechBubbleFromEnemy()` の引数に `SpeechBubbleConfig` を追加
- [x] 距離計算ロジック実装
- [x] サイズ選択 + スプライト選択ロジック実装
- [x] `sizeCategory` を `SpeechBubbleComponent` に設定

---

### 2-5. GameScene に SpeechBubbleConfig のロードを追加

- [x] `GameScene::Setup()` で `SpeechBubbleDataIO::Load()` を呼び出す
- [x] `CBSpeechBubbleConfigTag` エンティティに `SpeechBubbleConfig` コンポーネントとして追加

---

### 2-6. ImGui エディタ + JSON 保存

追加場所: `PlayerInfoDebugSystem` または専用エディタウィンドウ

```
[Speech Bubble Config]
  Distance: Large[___] Medium[___]

  [Large]  Size: X[___] Y[___]  Power:[___]  Duration:[___]
  [Medium] Size: X[___] Y[___]  Power:[___]  Duration:[___]
  [Small]  Size: X[___] Y[___]  Power:[___]  Duration:[___]
  [Save]
```

- [x] ImGui ウィンドウ実装 (EnemyDebug ウィンドウ内 CollapsingHeader)
- [x] `SpeechBubbleDataIO::Save()` 呼び出し
- [x] 初期 JSON ファイルを作成して動作確認 (起動時に自動生成/デフォルト値)

---

### 2-7. GameResourceComponent にスプライトパスを設定

**対象:** `GameResourceComponent.h` の `InitializeCommentBoutGameResources()`

```cpp
// kSpeechBubbleLarge → POW.png
// kSpeechBubbleMedium → BOOM.png  (旧 kAttackOrbEffectMedium と同じ)
// kSpeechBubbleSmall → OH.png
```

- [x] キー定数追加 (kSpeechBubbleLarge / Medium / Small)
- [x] テクスチャロード処理を `InitializeCommentBoutGameResources()` に追加
- [x] 旧 `kAttackOrbEffectXxx` を新キーのエイリアスに変更 (後方互換)

---

### Phase 2 完了確認

- [x] ビルドエラーなし
- [x] 敵を倒すと吹き出しが出る (3サイズ確認)
- [x] カメラ近距離: 大 (POW.png)、中距離: 中 (BOOM.png)、遠距離: 小 (OH.png)
- [x] ボスへのダメージ量がサイズで変わる
- [x] ImGui で距離閾値・サイズ・ダメージを変更し、JSON 保存できる

---

## Phase 3 — E: StageData 構造整備

> **目的:** データファイルをステージ依存/非依存で分類し、StageData.json ラッパーで管理する。
> **前提:** Phase 1・2 で Config ファイルのパスが変わっている前提

---

### 3-1. 新しいフォルダ構造の作成

```
resources/game/td_3105/
├── Data/                          ← 新規 (既存フォルダあり、中を整理)
│   ├── Config/                    ← 新規 (ステージ非依存)
│   │   ├── PlayerConfig.json      ← 移動 (Phase 1 で既に新パス設定済み)
│   │   ├── EnemyConfig.json       ← 移動 (Phase 1 で既に新パス設定済み)
│   │   ├── SpeechBubbleConfig.json ← 新規 (Phase 2 で作成済み)
│   │   └── FieldObjectTypeDefaults.json ← 移動
│   └── StageData/                 ← 新規
│       └── Stage_01/              ← 新規
│           ├── StageData.json     ← 新規 (ラッパー)
│           ├── RailData/
│           │   └── Stage_01_rail.json        ← 移動
│           ├── EventData/
│           │   └── Stage_01_events.json      ← 移動
│           └── FieldObjectData/
│               ├── Stage_01_field_objects.json ← 移動
│               └── Stage_01_hpbar.json          ← 移動
└── RailData/   ← 旧フォルダ (移行後に空になる。後で削除)
    └── sample_rail.json  ← 移動先検討 (サンプルなので Data/StageData/Sample/ へ)
```

- [x] `resources/game/td_3105/Data/Config/` フォルダ作成
- [x] `resources/game/td_3105/Data/StageData/Stage_01/RailData/` 作成
- [x] `resources/game/td_3105/Data/StageData/Stage_01/EventData/` 作成
- [x] `resources/game/td_3105/Data/StageData/Stage_01/FieldObjectData/` 作成

---

### 3-2. ファイル移動

- [x] `RailData/PlayerConfig.json` → `Data/Config/PlayerConfig.json`
- [x] `RailData/EnemyTypePreset.json` → `Data/Config/EnemyConfig.json` (リネームも)
- [x] `RailData/FieldObjectTypeDefaults.json` → `Data/Config/FieldObjectTypeDefaults.json`
- [x] `RailData/Stage_01_rail.json` → `Data/StageData/Stage_01/RailData/Stage_01_rail.json`
- [x] `RailData/Stage_01_events.json` → `Data/StageData/Stage_01/EventData/Stage_01_events.json`
- [x] `RailData/Stage_01_field_objects.json` → `Data/StageData/Stage_01/FieldObjectData/Stage_01_field_objects.json`
- [x] `RailData/Stage_01_hpbar.json` → `Data/StageData/Stage_01/FieldObjectData/Stage_01_hpbar.json`
- [ ] `RailData/sample_rail.json` → `Data/StageData/Sample/sample_rail.json` (ファイルが存在しないためスキップ)

---

### 3-3. StageData.json ラッパーを作成

**新規:** `resources/game/td_3105/Data/StageData/Stage_01/StageData.json`

```json
{
  "stageName": "Stage_01",
  "railDataPath":        "resources/game/td_3105/Data/StageData/Stage_01/RailData/Stage_01_rail.json",
  "eventDataPath":       "resources/game/td_3105/Data/StageData/Stage_01/EventData/Stage_01_events.json",
  "fieldObjectDataPath": "resources/game/td_3105/Data/StageData/Stage_01/FieldObjectData/Stage_01_field_objects.json",
  "hpBarDataPath":       "resources/game/td_3105/Data/StageData/Stage_01/FieldObjectData/Stage_01_hpbar.json"
}
```

- [ ] `StageData.json` を手動作成

---

### 3-4. StageDataIO クラスを作成

**新規:** `Data/StageDataIO.h/.cpp`

```cpp
struct StageData {
    std::string stageName;
    std::string railDataPath;
    std::string eventDataPath;
    std::string fieldObjectDataPath;
    std::string hpBarDataPath;
};

class StageDataIO {
public:
    static StageData Load(const std::string& stageDataJsonPath);
    static void Save(const StageData& data, const std::string& path);
    static std::string MakeStageDataPath(const std::string& stageName);
    // → "resources/game/td_3105/Data/StageData/{stageName}/StageData.json"
};
```

- [ ] `Data/StageDataIO.h/.cpp` 新規作成
- [ ] `vcxproj` / `vcxproj.filters` 更新

---

### 3-5. 既存 IO クラスのパス更新

全 IO クラスが新しいパスを参照するよう変更:

| IO クラス | 変更箇所 |
|----------|---------|
| `RailDataIO.cpp` | `MakeRailFilePath()` / `MakeEventFilePath()` を StageData から取得するように変更 |
| `PlayerDataIO.cpp` | ハードコードパスを `Data/Config/PlayerConfig.json` に変更 |
| `EnemyDataIO.cpp` | ハードコードパスを `Data/Config/EnemyConfig.json` に変更 |
| `FieldEditorSystem.cpp` | フィールドオブジェクト・デフォルトパスを新パスに変更 |

- [x] `RailDataIO.cpp` の `MakeRailFilePath()` / `MakeEventFilePath()` を更新
- [x] `PlayerDataIO` のパス定数を更新
- [x] `EnemyDataIO` のパス定数を更新
- [x] `FieldEditorSystem.cpp` の `FieldObjectTypeDefaults.json` パスを更新
- [x] `HpBarViewSystem` (hpbar.json 読み込み) のパスを新パスに変更 (ハードコード)

---

### 3-6. GameScene で StageData を使用

`GameScene::Setup()` または `RailEditorImGui()` で:
```cpp
StageData stageData = StageDataIO::Load(StageDataIO::MakeStageDataPath("Stage_01"));
LoadRailToComponent(*rail, stageData.railDataPath);
LoadEventsToComponent(*rail, stageData.eventDataPath);
// ...
```

- [ ] `GameScene` に `StageDataIO` を使った読み込みを統合
- [ ] `RailEditorImGui()` の Load/Save ボタンを新パスに対応させる

---

### Phase 3 完了確認

- [x] ビルドエラーなし
- [x] ゲーム起動時に全 JSON が新パスで読み込まれる
- [x] エディタの Load/Save ボタンが正しいパスに書き込む
- [x] 旧 `RailData/` フォルダのファイルが不要になっている
- [x] ゲームがフルプレイ可能な状態を維持している

---

## Phase 4 — A: コントローラー対応

> **目的:** キーボード操作をコントローラーでも行えるようにする。
> **前提:** `Input::Pad` クラスが既に実装済み
> **ボタン対応表:**

| キーボード | ゲームパッド | 用途 |
|-----------|------------|------|
| SPACE | B ボタン | 決定・攻撃 |
| TAB | Start (≡) | ポーズ開閉 |
| W / ↑ | 左スティック上 + D-Pad Up | 上移動・カーソル上 |
| S / ↓ | 左スティック下 + D-Pad Down | 下移動・カーソル下 |
| A / ← | 左スティック左 + D-Pad Left | 左移動・カーソル左 |
| D / → | 左スティック右 + D-Pad Right | 右移動・カーソル右 |

> **スティックの閾値:** デッドゾーン 0.3f 推奨

---

### 4-1. 入力ヘルパーを作成 (任意)

**新規:** `Utility/InputHelper.h`

```cpp
namespace InputHelper {
    constexpr float kStickDeadZone = 0.3f;

    inline bool IsConfirmTrigger() {
        return No::Keyboard::IsTrigger(VK_SPACE) || No::Pad::IsTrigger(GamepadButton::B);
    }
    inline bool IsPauseTrigger() {
        return No::Keyboard::IsTrigger(VK_TAB) || No::Pad::IsTrigger(GamepadButton::Start);
    }
    inline bool IsMoveUpTrigger() {
        return No::Keyboard::IsTrigger('W') || No::Keyboard::IsTrigger(VK_UP)
            || No::Pad::IsTrigger(GamepadButton::Up);
    }
    inline bool IsMoveDownTrigger() {
        return No::Keyboard::IsTrigger('S') || No::Keyboard::IsTrigger(VK_DOWN)
            || No::Pad::IsTrigger(GamepadButton::Down);
    }
    inline bool IsMoveUpPress() {
        float ly = No::Pad::GetStick().leftStickY;
        return No::Keyboard::IsPress('W') || No::Keyboard::IsPress(VK_UP)
            || No::Pad::IsPress(GamepadButton::Up) || ly > kStickDeadZone;
    }
    inline bool IsMoveDownPress() {
        float ly = No::Pad::GetStick().leftStickY;
        return No::Keyboard::IsPress('S') || No::Keyboard::IsPress(VK_DOWN)
            || No::Pad::IsPress(GamepadButton::Down) || ly < -kStickDeadZone;
    }
    // 同様に Left / Right も
    inline No::Vector2 GetMoveStick() {
        float lx = No::Pad::GetStick().leftStickX;
        float ly = No::Pad::GetStick().leftStickY;
        // WASD でも Vector2 を生成
        float kbX = (No::Keyboard::IsPress('D') || No::Keyboard::IsPress(VK_RIGHT)) ? 1.f
                  : (No::Keyboard::IsPress('A') || No::Keyboard::IsPress(VK_LEFT))  ? -1.f : 0.f;
        float kbY = (No::Keyboard::IsPress('W') || No::Keyboard::IsPress(VK_UP))    ? 1.f
                  : (No::Keyboard::IsPress('S') || No::Keyboard::IsPress(VK_DOWN))  ? -1.f : 0.f;
        float x = (std::abs(lx) > kStickDeadZone) ? lx : kbX;
        float y = (std::abs(ly) > kStickDeadZone) ? ly : kbY;
        return { x, y };
    }
}
```

- [x] `Utility/InputHelper.h` 新規作成 (または既存 `CBGameAudio.h` と同じ場所に)
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 4-2. PauseSystem にパッド入力を追加

**対象:** `System/OutGame/PauseSystem.cpp`

検索キーワード: `VK_TAB`, `VK_SPACE`, `VK_UP`, `VK_DOWN`, `VK_LEFT`, `VK_RIGHT`

変更方針: 各 `Keyboard::IsTrigger(...)` の条件に `|| InputHelper::IsXxx()` を追加

- [x] ポーズ開閉 (`VK_TAB`) → `IsPauseTrigger()` に変更
- [x] 決定 (`VK_SPACE`) → `IsConfirmTrigger()` に変更
- [x] カーソル上下 → `IsMoveUpTrigger()` / `IsMoveDownTrigger()` に変更

---

### 4-3. OptionSystem にパッド入力を追加

**対象:** `System/OutGame/OptionSystem.cpp`

- [x] 決定 (`VK_SPACE`) → `IsConfirmTrigger()`
- [x] カーソル上下左右 → InputHelper に変更
- [ ] バック/キャンセル操作があれば A ボタンまたは B ボタンを対応付け (未実装・現状不要)

---

### 4-4. PlayerControlSystem にスティック入力を追加

**対象:** `System/PlayerControlSystem.cpp`

現在 WASD でプレイヤー移動している箇所に左スティック入力を追加:
```cpp
No::Vector2 input = InputHelper::GetMoveStick();
// 既存: if (Keyboard::IsPress('W')) input.y += 1.f; ... など
// 変更: Vector2 input = InputHelper::GetMoveStick();
```

- [x] `PlayerControlSystem.cpp` のキーボード入力を `InputHelper::GetMoveInput()` に統合
- [x] 攻撃ボタン (SPACE → A ボタン) も対応

---

### 4-5. その他キーボード入力箇所を確認

grep で `Keyboard::IsTrigger` / `Keyboard::IsPress` を全検索し、ゲームプレイに関わる箇所を全てパッド対応させる

- [x] grep `Keyboard::IsTrigger` を全体検索し、対応漏れがないか確認 (TitleSystem も対応済み)
- [ ] ClearOver 演出のメニュー操作 (Phase 7 で実装する箇所) も最初からパッド対応で実装

---

### Phase 4 完了確認

- [x] コントローラー接続時にポーズが Start ボタンで開閉できる
- [x] A ボタンで決定できる (計画では B ボタンだったが A ボタンに変更)
- [x] 左スティック / D-Pad でカーソル移動・プレイヤー移動ができる (スティックリピート付き)
- [x] キーボード操作も引き続き動作する
- [x] コントローラー未接続時のエラーが出ない (`IsGamepadConnected()` で確認)

---

## Phase 5 — G: エディタ管理クラス

> **目的:** 既存エディタを EditorManager に統合し、エディタモード切り替え + エンジンポーズを行う。
> **前提:** Phase 3 完了 (ファイルパスが安定している)
> **影響ファイル:**
> - 新規: `Editor/EditorManager.h/.cpp`
> - 新規: `FieldObject/Editor/FieldObjectEditor.h/.cpp`
> - 変更: `FieldObject/System/FieldEditorSystem.h/.cpp` (ImGui 分離)
> - 変更: `Scene/GameScene.h/.cpp`

---

### 5-0. Phase 2.5 で追加した個別エディタ System の統合

Phase 2.5 で以下のエディタ System が追加済み。Phase 5 で EditorManager のタブに統合する:
- `EnemyConfigEditorSystem` → "敵プリセット設定" ウィンドウ
- `SpeechBubbleConfigEditorSystem` → "吹き出し設定" ウィンドウ

統合時は `EditorManager::Update()` のタブに移動し、個別 System として GameScene への AddSystem は削除する。

### 5-1. FieldObjectEditor クラスを新規作成 (ImGui 分離)

**現状:** `FieldObject/System/FieldEditorSystem.cpp` に ImGui コードが混在
**目標:** ImGui 描画ロジックを `FieldObject/Editor/FieldObjectEditor` に移す

```
FieldEditorSystem (残す機能):
  - LoadFieldPlacements()
  - SaveFieldPlacements()
  - CreateFieldObjectEntity()
  - ApplyFieldObjectVisual()
  - ApplyFieldObjectCollision()
  - Update() ← ゲーム内のフィールドオブジェクト更新

FieldObjectEditor (移す機能):
  - DrawImGui(Registry&) ← 全 ImGui ウィンドウ描画
  - AddObjectImGui() など (現在 FieldEditorSystem 内の ImGui 関数)
```

- [x] `FieldObject/Editor/FieldObjectEditor.h/.cpp` 新規作成
- [x] `FieldEditorSystem` の ImGui 描画コードを `FieldObjectEditor` へ移動
- [x] `FieldEditorSystem` は `FieldObjectEditor*` をコンストラクタで受け取る形に変更
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 5-2. EditorManager クラスを新規作成

**新規:** `Editor/EditorManager.h/.cpp`

```cpp
class EditorManager {
public:
    void Initialize(No::Registry* registry, No::Entity railCameraEntity);
    void Update(No::Registry* registry, No::Entity railCameraEntity, float deltaTime);
    void SetEditorMode(bool enabled, No::Registry* registry);
    bool IsEditorMode() const { return isEditorMode_; }

private:
    bool isEditorMode_ = false;

    // 既存エディタ
    RailCameraEditor railCameraEditor_;
    GameEventEditor  gameEventEditor_;
    FieldObjectEditor fieldObjectEditor_;  // Phase 5-1 で作成

    char stageNameBuffer_[64] = "Stage_01";

    // スポーン位置可視化
    std::vector<No::Entity> spawnDebugEntities_;
    void CreateSpawnDebugEntities(No::Registry* registry, No::Entity railCameraEntity);
    void DestroySpawnDebugEntities(No::Registry* registry);

    No::PauseComponent* FindEnginePause(No::Registry* registry);
};
```

- [x] `Editor/EditorManager.h/.cpp` 新規作成
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 5-3. エディタモード切り替え実装 ※Phase 5.5 で設計変更

> **変更:** F1 トグル / SetEditorMode() / isEditorMode_ は Phase 5.5 で廃止。
> 代わりに常時表示 + "ゲームポーズ" チェックボックスを採用。

- [x] ~~`SetEditorMode()` 実装~~ → Phase 5.5 で廃止し "ゲームポーズ" チェックボックスに置換
- [x] エンジンポーズ ON/OFF の動作 (チェックボックス方式で確認)

---

### 5-4. エディタ ImGui ウィンドウをタブで統合

`EditorManager::Update()` に:
```cpp
// トグルボタン (F1 キーまたは ImGui ボタン)
if (ImGui::Button(isEditorMode_ ? "Exit Editor Mode" : "Enter Editor Mode")) {
    SetEditorMode(!isEditorMode_, registry);
}

if (isEditorMode_) {
    if (ImGui::BeginTabBar("EditorTabs")) {
        if (ImGui::BeginTabItem("Rail Editor")) {
            railCameraEditor_.DrawRailCameraImGui(registry, railCameraEntity);
            railCameraEditor_.DrawRailEditorImGui(registry, railCameraEntity);
            // Stage IO (Load/Save Rail, Load/Save Events) もここに移動
            EndTabItem();
        }
        if (ImGui::BeginTabItem("Event Editor")) {
            gameEventEditor_.DrawGameEventEditorImGui(registry, railCameraEntity);
            EndTabItem();
        }
        if (ImGui::BeginTabItem("Field Editor")) {
            fieldObjectEditor_.DrawImGui(registry);
            EndTabItem();
        }
        EndTabBar();
    }
}
```

- [x] タブ UI 実装 (レール / イベント / フィールド / 敵設定 / 吹き出し)
- [x] Stage IO ボタン (Load/Save Rail, Events) をレールタブ内に移動
- [x] EnemyConfigEditorSystem の ImGui を 敵設定タブに統合
- [x] SpeechBubbleConfigEditorSystem の ImGui を 吹き出しタブに統合

---

### 5-5. スポーン位置の可視化 (黒色モデル表示)

`CreateSpawnDebugEntities()`:
1. `RailCameraComponent.events` を全走査
2. `SpawnEnemy` タイプのイベントを抽出
3. 各スポーンイベントに対して:
   - `registry.GenerateEntity()` でデバッグエンティティ生成
   - `No::TransformComponent` に `spawnPosition` を設定
   - `moveDirection` を向くように rotation を設定
     ```
     Vector3 forward = normalize(moveDirection)
     rotation = QuaternionLookAt(forward, Vector3::UP)
     ```
   - 敵タイプに対応したモデルキーを使用:
     - `RailEnemyType::Boss` → `kBossModel`
     - その他 → `kEnemyModel`
   - `No::MeshComponent` を追加 + 黒マテリアル設定:
     - Albedo = (0, 0, 0, 1)
     - Emission = (0, 0, 0)

`DestroySpawnDebugEntities()`:
- `spawnDebugEntities_` の全エンティティを `registry.DestroyEntity()` で削除
- `spawnDebugEntities_` をクリア

- [x] `CreateSpawnDebugEntities()` 実装 (SpawnEnemy イベント × count 分の黒モデルを生成)
- [x] `DestroySpawnDebugEntities()` 実装
- [x] 黒マテリアル: `material->color = No::Color(0,0,0,1)`
- [x] `moveDirection` → `Quaternion::LookRotation(dir, Vector3::UP)`

---

### 5-6. GameScene を EditorManager に移行

`GameScene` の変更:
- `RailCameraEditor railCameraEditor_` → `EditorManager editorManager_` に統合
- `GameEventEditor gameEventEditor_` → EditorManager に統合
- `NotSystemUpdate()` を整理

```cpp
// GameScene.h に追加
EditorManager editorManager_;

// GameScene::Setup() に追加
editorManager_.Initialize(GetRegistry(), railCameraEntity_);

// GameScene::NotSystemUpdate() を変更
void GameScene::NotSystemUpdate() {
    CameraImGui();
    editorManager_.Update(GetRegistry(), railCameraEntity_, deltaTime_);
    ChangeSceneImGui();
}
```

- [x] `GameScene.h` の `railCameraEditor_` / `gameEventEditor_` を `editorManager_` に置き換え
- [x] `GameScene::Setup()` に `editorManager_.Initialize()` 追加
- [x] `GameScene::NotSystemUpdate()` を簡潔化 (`editorManager_.DrawImGui(*GetRegistry())` のみ)
- [x] 古い `RailEditorImGui()` / `RailCameraImGui()` メソッドを削除 (EditorManager 内に移動済み)
- [x] `EnemyConfigEditorSystem` / `SpeechBubbleConfigEditorSystem` を AddSystem から削除

---

### Phase 5 完了確認

- [x] EditorManager のタブ UI で Rail / Event / Field / 敵設定 / 吹き出し エディタが切り替えられる
- [x] スポーン位置に黒モデルが表示される (showSpawnDebug_ チェック時)
- [x] EnemyConfigEditorSystem / SpeechBubbleConfigEditorSystem を AddSystem から削除し EditorManager に統合
- [x] 既存の Load/Save 機能が EditorManager 経由で動作する

> **注:** Phase 5.5 で EditorManager の設計を大幅変更。エディタモード(F1 トグル)は廃止し常時表示に変更。詳細は Phase 5.5 を参照。

---

## Phase 5.5 — G2: エディタ大幅リファクタリング

> **目的:** Phase 5 で作成した EditorManager を全面再設計。エディタモード概念を廃止し、常時表示 + 手動ポーズ方式に移行。デバッグ機能の強化とレールデバッグ描画を専用 System に分離。
> **完了日:** 2026-03-29
> **影響ファイル:**
> - 変更: `Editor/EditorManager.h/.cpp` (全面リワーク)
> - 変更: `Editor/RailCameraEditor.h/.cpp` (インラインコンテンツメソッド追加)
> - 変更: `FieldObject/Editor/FieldObjectEditor.h/.cpp` (DrawTypeDefaultsImGui / ForceReload / Save 追加)
> - 変更: `FieldObject/System/FieldEditorSystem.h/.cpp` (SetStopInPause(false) 追加)
> - 新規: `System/RailCameraEditorSystem.h/.cpp` (レールデバッグ描画専用 System)
> - 変更: `System/RailCameraSystem.cpp` (デバッグ描画呼び出しを削除)
> - 変更: `Scene/GameScene.h/.cpp` (RailCameraEditorSystem 追加)
> - 変更: `Component/OutGame/PauseStateComponent.h` (`editorForcePause` フィールド確認済み)

---

### 5.5-1. エディタモード廃止・常時表示化

**変更内容:**
- F1 トグル / `isEditorMode_` / `SetEditorMode()` を全廃
- `EditorManager::DrawImGui()` は常時 `ImGui::Begin("Editor##EditorManager")` ウィンドウを表示
- ゲームポーズは手動チェックボックスで制御

**ポーズチェックボックスの実装方針:**
- `ImGui::Checkbox("ゲームポーズ", &isPauseEnabled_)` が変更を返したときだけ `PauseComponent::isPause` を書き換える
- `PauseSystem` が isPause=true 時に停止するため、チェック ON 中は PauseSystem も止まり競合しない
- `PauseStateComponent::editorForcePause` に状態を永続保存 (フレーム開始時に同期読み込み)

- [x] エディタモード (F1 / isEditorMode_) 廃止
- [x] `DrawImGui()` の常時表示化
- [x] ゲームポーズチェックボックス実装 (`isPauseEnabled_` + `PauseComponent::isPause`)
- [x] `PauseStateComponent::editorForcePause` との同期

---

### 5.5-2. ステージ名入力 + すべて読込/保存ボタン

**EditorManager のウィンドウ上部:**
```
[ステージ名: ________] [すべて読込] [すべて保存]
```

- `LoadAll(registry)`: rail->stageName 設定 → LoadRailData → LoadRailEvents → `fieldObjectEditor_.ForceReload()`
- `SaveAll(registry)`: SaveRailData → SaveRailEvents → `fieldObjectEditor_.Save(registry)`
- `fieldObjectEditor_.ForceReload()` は `loadedRegistry_ = nullptr; loadedStageName_ = ""` をクリアし次フレームで自動再読込させる

- [x] `stageNameBuffer_[64]` + `ImGui::InputText` 実装
- [x] "すべて読込" / "すべて保存" ボタン実装
- [x] `LoadAll()` / `SaveAll()` 実装

---

### 5.5-3. デバッグ表示 CollapsingHeader

`ImGui::CollapsingHeader("デバッグ表示")` 内に以下を実装:

**スポーンデバッグ:**
- `ImGui::Checkbox("スポーンデバッグ", &showSpawnDebug_)` — チェック変更時に CreateSpawnDebugEntities / DestroySpawnDebugEntities
- showSpawnDebug_ が true の間、毎フレーム `ComputeSpawnEventSignature()` でシグネチャを計算し変化検知時に再生成 (イベント編集中のリアルタイム更新)

```cpp
// シグネチャ計算例
int sig = static_cast<int>(rail->events.size()) * 97;
for (SpawnEnemy events: ...) { sig += count*31 + posX*7 + posY*11 + posZ*13 + ...; }
```

**レールデバッグ:**
- `ImGui::Checkbox("レール描画", &rail->drawRailDebug)`
- `ImGui::Checkbox("カメラギズモ", &rail->drawCameraDebug)`
- `ImGui::Checkbox("制御点", &rail->drawControlPointsDebug)`
- `ImGui::Checkbox("イベント点", &rail->drawEventPointsDebug)`
- `ImGui::DragFloat("デバッグ半径", ...)` — `controlPointDebugRadius` と `eventPointDebugRadius` を同時更新

- [x] スポーンデバッグチェックボックス + 変化検知リアルタイム再生成
- [x] `ComputeSpawnEventSignature()` 実装
- [x] レールデバッグチェックボックス群実装 (RailCameraComponent のフラグを直接操作)
- [x] "デバッグ半径" DragFloat (controlPointDebugRadius / eventPointDebugRadius 共有)

---

### 5.5-4. タブ構成変更 (フィールドオブジェクト設定タブ分離)

**タブ一覧 (順序固定):**

| タブ名 | 内容 |
|--------|------|
| レール | LoadRail/SaveRail/LoadEvents/SaveEventsボタン + RailCameraContent (インライン) + CollapsingHeader"制御点編集" |
| イベント | gameEventEditor_.DrawGameEventEditorImGui (※フローティングウィンドウを開く) |
| フィールド | fieldObjectEditor_.DrawImGui (配置UI、種別設定は別タブへ) |
| 敵設定 | EnemyConfig の ImGui (Phase 5 から引き継ぎ) |
| フィールドオブジェクト設定 | fieldObjectEditor_.DrawTypeDefaultsImGui (種別デフォルト設定) |
| 吹き出し | SpeechBubble の ImGui (Phase 5 から引き継ぎ) |

- [x] "フィールドオブジェクト設定" タブを "敵設定" の後に新設
- [x] `FieldObjectEditor::DrawTypeDefaultsImGui()` をそのタブから呼ぶ
- [x] フィールドタブから種別デフォルト設定 UI を除去し `ImGui::TextDisabled` ヒントを表示

---

### 5.5-5. RailCameraEditor インラインコンテンツメソッド追加

**変更前:** `DrawRailCameraImGui` / `DrawRailEditorImGui` (ImGui::Begin/End 付きフローティング)
**変更後:** 以下を追加 (Begin/End なし、タブ内インライン用)

```cpp
void DrawRailCameraContent(No::Registry*, No::Entity, ResetEventRuntimeFn);  // 再生/停止/進行率UI
void DrawRailEditorContent(No::Registry*, No::Entity);                        // 制御点編集UI
```

- [x] `DrawRailCameraContent()` 追加 (Begin/End なし版)
- [x] `DrawRailEditorContent()` 追加 (Begin/End なし版、BeginChild 高さ 200)
- [x] `EditorManager::DrawRailTab()` からこれらを呼ぶよう変更

---

### 5.5-6. RailCameraEditorSystem 新規作成

**目的:** レール/制御点/イベント点/カメラギズモのデバッグ描画を `RailCameraSystem` から分離し、ポーズ中も描画が継続するようにする。

**重要:** `SetStopInPause(false)` はコンストラクタ内で呼ぶこと (NoEngine の仕様)。
ポーズ中もデバッグ描画が必要な全システムに適用が必要。

```cpp
// System/RailCameraEditorSystem.h
class RailCameraEditorSystem : public No::ISystem {
public:
    RailCameraEditorSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
};
```

- `Update()` は `View<RailCameraComponent, TransformComponent>` でイテレートし、
  `rail->drawRailDebug` 等のフラグに応じて DrawRailDebug / DrawControlPointDebug / DrawEventPointDebug / DrawRailCameraGizmo を呼ぶ
- ヘルパー関数 (InterpolateSplinePoint, EvaluatePositionByT, DistanceToNormalizedT) は anonymous namespace 内に複製

**RailCameraSystem.cpp の変更:**
- DrawRailDebug / DrawControlPointDebug / DrawEventPointDebug / DrawRailCameraGizmo の呼び出しを削除
- ヘルパー関数は RailCameraSystem 側でも保持 (計算用)

**FieldEditorSystem の変更:**
- コンストラクタ内に `SetStopInPause(false)` 追加 (フィールド配置もポーズ中に更新が必要なため)

- [x] `System/RailCameraEditorSystem.h/.cpp` 新規作成
- [x] `SetStopInPause(false)` をコンストラクタで設定
- [x] デバッグ描画の 4 関数を RailCameraEditorSystem.cpp に移植
- [x] `RailCameraSystem.cpp` からデバッグ描画呼び出しを削除
- [x] `FieldEditorSystem` コンストラクタに `SetStopInPause(false)` 追加
- [x] `GameScene::Setup()` に `AddSystem(make_unique<RailCameraEditorSystem>())` 追加

---

### 5.5-7. vcxproj / filters 更新

**追加エントリ:**

| 種別 | パス |
|------|------|
| ClCompile | `application\CommentBout\System\RailCameraEditorSystem.cpp` |
| ClInclude | `application\CommentBout\System\RailCameraEditorSystem.h` |

(EditorManager.cpp / FieldObjectEditor.cpp の ClCompile は Phase 5 で追加済み)

- [x] `NoEngine.vcxproj` に RailCameraEditorSystem エントリ追加
- [x] `NoEngine.vcxproj.filters` に RailCameraEditorSystem エントリ追加

---

### Phase 5.5 完了確認

- [x] エディタウィンドウが常時表示される (F1 不要)
- [x] "ゲームポーズ" チェックボックスでゲームを停止/再開できる
- [x] ステージ名入力後に "すべて読込/保存" でレール+イベント+フィールドを一括操作できる
- [x] デバッグ表示セクションでスポーンデバッグとレールデバッグを個別に ON/OFF できる
- [x] イベントを編集中にスポーンデバッグが自動更新される (シグネチャ変化検知)
- [x] ポーズ中もレールデバッグ描画が継続する (RailCameraEditorSystem が SetStopInPause(false))
- [x] "フィールドオブジェクト設定" タブが "敵設定" の後に存在する

> **未解決:** GameEventEditor の DrawGameEventEditorImGui は現状フローティングウィンドウを開く。
> イベントタブからフローティングウィンドウが開くため UX が若干不整合。次の機会にインライン化を検討。

---

## Phase 6 — H前半: ボス構造変更

> **目的:** ボスの位置計算をレールカメラ基準に変更し、ゲーム開始時から存在するようにする。
> **前提:** Phase 5.5 完了 (EditorManager + GameEventEditor でイベントデータを編集可能)
> **影響ファイル:**
> - `System/BossBehaviorSystem.cpp`
> - `resources/game/td_3105/Data/StageData/Stage_01/EventData/Stage_01_events.json`
>   (EditorManager の "イベント" タブ → GameEventEditor ウィンドウから編集可能)
>
> **アーキテクチャ注記 (Phase 5.5 後):**
> - `EditorManager::DrawImGui()` は `GameScene::NotSystemUpdate()` から呼ばれる
> - ゲームシーン AddSystem 順: `...RailCameraSystem → RailCameraEditorSystem → FieldEditorSystem...`
> - `FieldEditorSystem` は `editorManager_.GetFieldObjectEditor()` のポインタを受け取る

---

### 6-1. BossBehaviorSystem をレールカメラ参照に変更

**現状:**
```cpp
// ActiveCameraTag を持つエンティティのカメラを使用
auto cameraView = registry.View<No::ActiveCameraTag, No::TransformComponent>();
```

**変更後:**
```cpp
// RailCameraComponent を持つエンティティのカメラを使用
auto cameraView = registry.View<RailCameraComponent, No::TransformComponent>();
```

これにより、デバッグカメラに切り替えてもボスがレールカメラ基準の位置に留まる。

- [x] `BossBehaviorSystem.cpp` のカメラ取得ロジックを `RailCameraComponent` 検索に変更
- [x] 他のボス関連システム (`BossPhase` の射撃ターゲット計算など) も同様に確認

---

### 6-2. ボスのスポーンを開始地点に変更

**方針:** `triggerDistance = 0.5` のスポーンイベントを追加 (案A)

`Stage_01_events.json` を編集:
- 既存のボススポーンイベント (`triggerDistance: 19.0`) を削除 or コメントアウト
- 新しいボススポーンイベントを追加 (`triggerDistance: 0.5`)

```json
{
  "type": "SpawnEnemy",
  "triggerDistance": 0.5,
  "spawn": {
    "count": 1,
    "enemyType": "Boss",
    "spawnPosition": [0.0, 1.5, 12.0],
    "moveDirection": [0.0, 0.0, -1.0],
    "spawnGroupId": 99,
    "boss": {
      "offsetLocal": [0.0, 0.20000000298023224, 9.5],
      "figure8Amplitude": [2.799999952316284, 1.399999976158142],
      "figure8Period": 4.5,
      "stopDuration": 1.0,
      "burstShotInterval": 0.2199999988079071
    }
  }
}
```

- [x] `Stage_01_events.json` を直接編集 (または EditorManager 完成後はエディタから)
- [ ] ゲーム起動直後にボスが適切な位置にスポーンするか確認
- [ ] レールカメラからの `offsetLocal` 値が正しく適用されるか確認

---

### 6-3. クリア/オーバー条件のトラッキング準備

**新規コンポーネント:** `Component/GameResultComponent.h`

```cpp
enum class GameResult { None, Clear, Over };

struct GameResultComponent {
    GameResult result = GameResult::None;

    // Over の理由
    bool playerDied = false;
    bool railReachedEnd = false;

    // デバッグ設定
    bool debugDisableClear = false;  // クリア条件を無効化
    bool debugDisableOver  = false;  // オーバー条件を無効化
};
```

- [x] `Component/GameResultComponent.h` 新規作成
- [x] `GameScene::Setup()` でシングルトン的にエンティティに追加

---

### 6-4. レール終端検知

**対象:** `System/RailCameraSystem.cpp` または新規 `GameResultSystem`

レールが最終地点まで進行したかを検知:
```cpp
if (rail->traveledDistance >= rail->GetTotalLength()) {
    gameResult->railReachedEnd = true;
    // GameResult::Over 判定は GameResultSystem に委ねる
}
```

- [x] `RailCameraSystem.cpp` にレール終端フラグ設定処理を追加 (または `GameResultSystem` で監視)
- [ ] レール終端判定のログ確認

---

### Phase 6 完了確認

- [ ] ゲーム開始直後にボスがスポーンしている
- [ ] デバッグカメラに切り替えてもボスが移動しない (レールカメラ基準を維持)
- [ ] `GameResultComponent` がシーンに存在する
- [ ] ボスの HP を 0 にするとクリア判定の準備が整っている (Phase 7 で完成)

---

## Phase 7 — H後半: クリア/オーバー演出

> **目的:** ボス撃破→クリア演出、プレイヤー死亡/レール終端→オーバー演出を実装する。
> **前提:** Phase 6 完了 (GameResultComponent, ボス構造変更済み)
> **参考:** `PauseStateComponent` / `PauseSystem` の構造を踏襲する

---

### 7-1. GameResultSystem 新規作成

**新規:** `System/GameResultSystem.h/.cpp`

役割: クリア/オーバー条件を監視して `GameResultComponent.result` を更新する

```
条件監視:
  クリア: CBBossTag + HealthComponent.isDead == true
         → debugDisableClear が false のとき result = GameResult::Clear

  オーバー: CBPlayerTag + HealthComponent.isDead == true
         → result = GameResult::Over

  オーバー: GameResultComponent.railReachedEnd == true
         → result = GameResult::Over

result が None 以外になったら:
  → RailCameraSystem を停止させるフラグを設定
  → BossDefeatSequence を開始 (クリアの場合)
  → ClearOverState をアクティブ化
```

- [x] `System/GameResultSystem.h/.cpp` 新規作成
- [x] `GameScene::Setup()` に追加 (適切な順序で)
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 7-2. ボス撃破演出コンポーネントとシステム

**新規:** `Component/BossDefeatSequenceComponent.h`

```cpp
struct BossDefeatSequenceComponent {
    enum class Phase {
        Inactive,
        PreExplosionWait,   // 爆発前の待機
        Explosion,           // BOOM スプライト表示中
        PostExplosionWait,   // 爆発後の余韻
        Done                 // 完了
    };

    Phase phase = Phase::Inactive;
    float phaseTimer = 0.f;

    // ImGui / JSON で設定可能
    float preExplosionDuration  = 0.3f;  // 爆発前待機時間
    float explosionDuration     = 0.8f;  // 爆発表示時間
    float postExplosionDuration = 0.5f;  // 爆発後待機時間

    No::Entity bossEntity;       // 消す対象ボスエンティティ
    No::Entity effectSpriteEntity = No::INVALID_ENTITY;  // BOOM スプライト
};
```

**新規:** `System/BossDefeatSystem.h/.cpp`

```
Update():
  BossDefeatSequenceComponent が Inactive 以外:
    PreExplosionWait:
      - プレイヤーを無敵にする (InvincibleComponent を長時間設定)
      - タイマー経過 → Explosion へ

    Explosion:
      - effectSpriteEntity が INVALID なら BOOM スプライトをボス位置に生成
      - タイマー経過 → ボスエンティティを DestroyEntity
                    → PostExplosionWait へ

    PostExplosionWait:
      - タイマー経過 → Done へ
                    → ClearOverStateComponent をアクティブ化 (Clear)
```

- [x] `Component/BossDefeatSequenceComponent.h` 新規作成
- [x] `System/BossDefeatSystem.h/.cpp` 新規作成
- [x] ボスの BOOM スプライト生成: ボスのスクリーン位置に `SpriteComponent` を追加
- [x] `GameResultSystem` が result=Clear を検知したら `BossDefeatSequenceComponent.phase = PreExplosionWait` に設定
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 7-3. ClearOverStateComponent を作成

**新規:** `Component/ClearOverStateComponent.h`

PauseStateComponent を参考に設計:

```cpp
struct ClearOverStateComponent {
    enum class Phase {
        Inactive,
        FadeIn,      // 暗幕が暗くなる
        LogoAppear,  // ロゴ出現 (イージング)
        MenuAppear,  // メニュー出現 (イージング)
        MenuActive,  // 選択中
    };

    enum class Result { Clear, Over };
    enum class Action { None, Restart, BackToTitle };

    Phase phase = Phase::Inactive;
    Result result = Result::Clear;
    Action requestedAction = Action::None;

    float phaseTimer = 0.f;
    int selectedIndex = 0;  // 0=リスタート, 1=タイトルへ

    bool isConfirmAnimating = false;
    float confirmAnimTime = 0.f;
};
```

**新規:** `Component/ClearOverConfigComponent.h`

```cpp
struct ClearOverConfigComponent {
    // フェード設定
    float fadeDuration    = 0.5f;
    float fadeTargetAlpha = 0.85f;

    // ロゴ設定
    No::Vector2 logoStartPos{ 640.f, 800.f };
    No::Vector2 logoEndPos  { 640.f, 250.f };
    No::Vector2 logoSize    { 400.f, 100.f };
    float logoAppearDuration = 0.4f;
    int   logoEaseType = 2;  // EaseOutCubic

    // メニュー設定 (ポーズと同様の構造)
    No::Vector2 itemBaseStartPos{ 640.f, 900.f };
    No::Vector2 itemBaseEndPos  { 640.f, 420.f };
    No::Vector2 itemSize        { 340.f, 70.f };
    float itemSpacing     = 80.f;
    float menuAppearDuration = 0.3f;

    // 確認アニメーション
    float confirmDuration = 0.12f;
    float selectedScale   = 1.08f;
    float confirmScale    = 1.18f;

    // ボス演出タイミング (クリア時のみ)
    float preExplosionDuration  = 0.3f;
    float explosionDuration     = 0.8f;
    float postExplosionDuration = 0.5f;
};
```

- [x] `Component/ClearOverStateComponent.h` 新規作成
- [x] `Component/ClearOverConfigComponent.h` 新規作成
- [x] JSON シリアライズ対応 (ClearOverConfig の各値を保存可能に)
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 7-4. ClearOverSystem 新規作成

**新規:** `System/ClearOverSystem.h/.cpp`

役割: フェーズ管理 + 入力処理 + シーン遷移

```
Update():
  ClearOverStateComponent が Inactive: return

  FadeIn:
    フェードエンティティの alpha を 0 → fadeTargetAlpha (EaseInSine)
    fadeDuration 経過 → LogoAppear へ

  LogoAppear:
    ロゴスプライトを startPos → endPos (EaseOutBack)
    logoAppearDuration 経過 → MenuAppear へ

  MenuAppear:
    メニュースプライトを startPos → endPos
    menuAppearDuration 経過 → MenuActive へ

  MenuActive:
    入力処理 (キーボード/パッド共通):
      Up/Down → selectedIndex 変更 (0↔1、SystemMoveCursor SE)
      確定 → confirmAnimating = true, requestedAction 設定 (SystemDecision SE)

    confirmAnimating 中:
      confirmAnimTime += deltaTime
      完了したら:
        Restart → ClearPauseState → SceneChangeEvent("GameScene")
        BackToTitle → SceneChangeEvent("TitleScene")
```

- [x] `System/ClearOverSystem.h/.cpp` 新規作成
- [x] SceneChangeEvent の発火を PauseSystem と同じパターンで実装
- [x] `GameScene::Setup()` に追加 (PauseSystem より後の順序で)
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 7-5. ClearOverViewSystem 新規作成

**新規:** `System/ClearOverViewSystem.h/.cpp`

役割: 演出スプライトの生成・更新・フェーズに応じた表示

```
初期化時 (フェーズが Inactive → FadeIn に変わったとき):
  1. フェードオーバーレイエンティティ生成
     - white1x1 テクスチャ、色 = (0, 0, 0, 0)、フルスクリーンサイズ (1280x720)
     - レイヤー: 高い値 (例: 200)

  2. ロゴエンティティ生成
     - Clear なら kClearLogo、Over なら kOverLogo テクスチャ
     - startPos 位置、alpha = 0

  3. メニューエンティティ生成 (2つ: リスタート, タイトルへ)
     - kRestart / kPauseToTitle テクスチャ
     - startPos 位置

毎フレーム:
  FadeIn:   フェードオーバーレイの alpha を lerp
  LogoAppear: ロゴ位置・alpha を lerp
  MenuAppear: メニュー位置を lerp
  MenuActive: selectedIndex に応じてカーソル位置・スケール更新

Inactive になったら全エンティティを DestroyEntity
```

- [x] `System/ClearOverViewSystem.h/.cpp` 新規作成
- [x] `GameResourceComponent.h` にキー追加 (kClearLogo / kOverLogo / kFadeOverlay)
- [x] `InitializeCommentBoutGameResources()` にロード処理追加
- [x] `vcxproj` / `vcxproj.filters` 更新

---

### 7-6. RailCamera の停止処理

クリア/オーバー確定時にレールを止める:

**対象:** `System/RailCameraSystem.cpp`

```cpp
// RailCameraSystem::Update() の冒頭に追加
auto* gameResult = ...; // GameResultComponent を取得
if (gameResult && gameResult->result != GameResult::None) {
    return; // レール更新停止
}
```

- [x] `RailCameraSystem::Update()` にゲーム結果チェックを追加

---

### 7-7. デバッグ設定 ImGui

`PlayerInfoDebugSystem` または専用の Debug ImGui ウィンドウに追加:
```
[Game Result Debug]
  Disable Clear: [checkbox]
  Disable Over:  [checkbox]
  [Force Clear]  [Force Over]  ← ボタンで強制実行
```

- [x] デバッグ用 ImGui 追加 (PlayerInfoDebugSystem 推奨)

---

### 7-8. ClearOverConfig を JSON 保存可能にする

- [x] `ClearOverConfig` の IO クラス or インライン save/load 実装 (ClearOverConfigDataIO)
- [x] JSON パス: `"resources/game/td_3105/Data/Config/ClearOverConfig.json"`
- [ ] ImGui ウィンドウ: フェード時間、ロゴ位置・サイズ、イージングタイプなど編集可能に (Phase 9 以降)

---

### Phase 7 完了確認

**クリア確認:**
- [ ] ボスの HP を 0 にすると爆発演出が始まる
- [ ] 爆発後にボスエンティティが消える
- [ ] レールが停止する
- [ ] 画面がフェードアウトする
- [ ] クリアロゴが出現する
- [ ] リスタート/タイトルボタンが出現して選択できる
- [ ] リスタートでゲームシーンが再ロードされる
- [ ] タイトルに戻るとタイトルシーンに遷移する
- [ ] コントローラーでも操作できる (Phase 4 対応済みなら自動)

**オーバー確認:**
- [ ] プレイヤー HP が 0 になるとオーバー演出が始まる
- [ ] レールが終端に到達してもオーバー演出が始まる
- [ ] レールが停止する
- [ ] 画面がフェードアウト → オーバーロゴ → メニューの流れが正常

**デバッグ確認:**
- [ ] debugDisableClear が true のとき、ボスを倒してもクリアにならない
- [ ] debugDisableOver が true のとき、HP が 0 になってもオーバーにならない

---

## Phase 8 — I: 音実装

> **目的:** 各アクションに対応する音を実装する。
> **前提:** 全 SE/BGM ファイルが `resources/game/td_3105/Audio/` に存在済み
> **方針:** 各 Phase の完了後に対応する音を追加していく

---

### 8-1. GameResourceComponent に音キーを追加

**対象:** `Component/GameResourceComponent.h`

```cpp
// BGM
static const char* kBGMInGame = "BGMInGame";  // InGameBGM01.mp3
static const char* kBGMTitle  = "BGMTitle";   // TitleBGM01.mp3

// SE - 戦闘
static const char* kSEBossDamage  = "SEBossDamage";   // BossDamage.mp3
static const char* kSEPlayerDamage = "SEPlayerDamage"; // PlayerDamage.mp3
static const char* kSEEnemyDamage  = "SEEnemyDamage";  // EnemyDamage.mp3
static const char* kSEEnemyShot    = "SEEnemyShot";    // EnemyShot.mp3
static const char* kSEBossShot     = "SEBossShot";     // BossShot.mp3
static const char* kSEPlayerPunch  = "SEPlayerPunch";  // PlayerPunch.mp3

// SE - 吹き出し
static const char* kSESpeechBubbleL = "SESpeechBubbleL";  // SpeechBubbleL.mp3
static const char* kSESpeechBubbleM = "SESpeechBubbleM";  // SpeechBubbleM.mp3
static const char* kSESpeechBubbleS = "SESpeechBubbleS";  // SpeechBubbleS.mp3

// SE - システム
static const char* kSESystemDecision  = "SESystemDecision";  // SystemDecision.mp3
static const char* kSESystemMoveCursor = "SESystemMoveCursor"; // SystemMoveCursor.mp3
static const char* kSESystemOpen       = "SESystemOpen";       // SystemOpen.mp3
```

- [ ] キー定数追加

---

### 8-2. CBGameAudio への登録

**対象:** `InitializeCommentBoutGameResources()` または専用の音初期化関数

```cpp
// BGM
CBGameAudio::RegisterBGMClip(kBGMInGame, L"resources/game/td_3105/Audio/BGM/InGameBGM01.mp3");
CBGameAudio::RegisterBGMClip(kBGMTitle,  L"resources/game/td_3105/Audio/BGM/TitleBGM01.mp3");

// SE - 戦闘
CBGameAudio::RegisterSEClip(kSEBossDamage,   L"resources/game/td_3105/Audio/SE/BossDamage.mp3");
CBGameAudio::RegisterSEClip(kSEPlayerDamage,  L"resources/game/td_3105/Audio/SE/PlayerDamage.mp3");
CBGameAudio::RegisterSEClip(kSEEnemyDamage,   L"resources/game/td_3105/Audio/SE/EnemyDamage.mp3");
CBGameAudio::RegisterSEClip(kSEEnemyShot,     L"resources/game/td_3105/Audio/SE/EnemyShot.mp3");
CBGameAudio::RegisterSEClip(kSEBossShot,      L"resources/game/td_3105/Audio/SE/BossShot.mp3");
CBGameAudio::RegisterSEClip(kSEPlayerPunch,   L"resources/game/td_3105/Audio/SE/PlayerPunch.mp3");
// SE - 吹き出し
CBGameAudio::RegisterSEClip(kSESpeechBubbleL, L"resources/game/td_3105/Audio/SE/SpeechBubbleL.mp3");
CBGameAudio::RegisterSEClip(kSESpeechBubbleM, L"resources/game/td_3105/Audio/SE/SpeechBubbleM.mp3");
CBGameAudio::RegisterSEClip(kSESpeechBubbleS, L"resources/game/td_3105/Audio/SE/SpeechBubbleS.mp3");
// SE - システム
CBGameAudio::RegisterSEClip(kSESystemDecision,   L"resources/game/td_3105/Audio/SE/SystemDecision.mp3");
CBGameAudio::RegisterSEClip(kSESystemMoveCursor,  L"resources/game/td_3105/Audio/SE/SystemMoveCursor.mp3");
CBGameAudio::RegisterSEClip(kSESystemOpen,        L"resources/game/td_3105/Audio/SE/SystemOpen.mp3");
```

- [ ] 全音声ファイルを登録
- [ ] ファイルパスの typo がないか確認 (実際のファイル名と照合)

---

### 8-3. BGM 再生

| タイミング | 場所 | 音 |
|-----------|------|-----|
| ゲームシーン開始 | `GameScene::Setup()` 最後 | InGameBGM01 (ループ) |
| タイトルシーン開始 | `TitleScene::Setup()` 最後 | TitleBGM01 (ループ) |
| クリア/オーバー確定 | `GameResultSystem` or `ClearOverSystem` | BGM 停止 |

- [ ] `GameScene::Setup()` に `CBGameAudio::PlayBGMClip(kBGMInGame, volume, true)` 追加
- [ ] `TitleScene::Setup()` に `CBGameAudio::PlayBGMClip(kBGMTitle, volume, true)` 追加
- [ ] クリア/オーバー時に BGM 停止

---

### 8-4. 戦闘 SE の実装

| タイミング | ファイル | 実装箇所 |
|-----------|---------|---------|
| ダメージ: ボス | BossDamage.mp3 | `DamageApplySystem.cpp` CBBossTag チェック後 |
| ダメージ: 自機 | PlayerDamage.mp3 | `DamageApplySystem.cpp` CBPlayerTag チェック後 |
| ダメージ: 敵 | EnemyDamage.mp3 | `DamageApplySystem.cpp` CBRailEnemyTag チェック後 |
| 弾発射: 敵 | EnemyShot.mp3 | `EnemyShootSystem.cpp` 通常敵の弾生成時 |
| 弾発射: ボス | BossShot.mp3 | `EnemyShootSystem.cpp` ボスの弾生成時 |
| 自機攻撃 | PlayerPunch.mp3 | `PlayerControlSystem.cpp` or `PlayerAttackResolveSystem.cpp` 攻撃生成時 |

- [ ] `DamageApplySystem.cpp` にタグ別 SE 再生を追加
  - CBBossTag: `CBGameAudio::PlaySEClip(kSEBossDamage, ...)`
  - CBPlayerTag: `CBGameAudio::PlaySEClip(kSEPlayerDamage, ...)`
  - CBRailEnemyTag (ボスでない): `CBGameAudio::PlaySEClip(kSEEnemyDamage, ...)`
- [ ] `EnemyShootSystem.cpp` の `SpawnEnemyBullet()` 呼び出し箇所でボス/通常を分けて SE 再生
- [ ] 自機攻撃時の SE 再生実装

---

### 8-5. 吹き出し SE の実装

**対象:** `EnemyVisualSystem.cpp` の `SpawnSpeechBubbleFromEnemy()`

```cpp
switch (sizeCategory) {
    case 0: CBGameAudio::PlaySEClip(kSESpeechBubbleL, ...); break;  // Large
    case 1: CBGameAudio::PlaySEClip(kSESpeechBubbleM, ...); break;  // Medium
    case 2: CBGameAudio::PlaySEClip(kSESpeechBubbleS, ...); break;  // Small
}
```

- [ ] サイズ別 SE 再生を `SpawnSpeechBubbleFromEnemy()` に追加

---

### 8-6. システム SE の実装

| タイミング | SE | 実装箇所 |
|-----------|-----|---------|
| ポーズ/オプション開く | SystemOpen | `PauseSystem.cpp` (ポーズ開始時) |
| ポーズ/オプション内カーソル移動 | SystemMoveCursor | `PauseSystem.cpp` / `OptionSystem.cpp` |
| ポーズ/オプション内決定 | SystemDecision | `PauseSystem.cpp` / `OptionSystem.cpp` |
| クリア/オーバーメニュー開く | SystemOpen | `ClearOverViewSystem.cpp` (MenuAppear時) |
| クリア/オーバーカーソル移動 | SystemMoveCursor | `ClearOverSystem.cpp` |
| クリア/オーバー決定 | SystemDecision | `ClearOverSystem.cpp` |

- [ ] `PauseSystem.cpp` にシステム SE 追加 (開く, カーソル, 決定)
- [ ] `OptionSystem.cpp` にシステム SE 追加
- [ ] `ClearOverSystem.cpp` にシステム SE 追加

---

### Phase 8 完了確認

- [ ] ゲームシーンで BGM が流れる (ループ再生)
- [ ] タイトルシーンで BGM が流れる
- [ ] クリア/オーバー時に BGM が止まる
- [ ] ボスにダメージを与えると BossDamage SE が鳴る
- [ ] 自機がダメージを受けると PlayerDamage SE が鳴る
- [ ] 敵を倒すと EnemyDamage SE が鳴る
- [ ] 吹き出し大/中/小で対応する SE が鳴る
- [ ] ポーズを開くと SystemOpen SE が鳴る
- [ ] カーソル移動で SystemMoveCursor SE が鳴る
- [ ] 決定で SystemDecision SE が鳴る

---

## Phase 9 — F: フォルダ精査・整理

> **目的:** 全機能の実装完了後、フォルダ構造・ファイル配置を見直し整理する。
> **前提:** Phase 1〜8 が全て完了している

---

### 9-1. 精査フェーズ (実装前に確認)

このフェーズに入る前に以下を確認・洗い出す:

- [ ] 全ソースファイルのフォルダ配置を一覧化
- [ ] Editor フォルダ: `RailCameraEditor`, `GameEventEditor`, `EditorManager`, `FieldObjectEditor` が揃っているか
- [ ] System フォルダ: 肥大化していないか、サブフォルダ分けが必要か
  - `OutGame/` に PauseSystem があるが、`ClearOverSystem` / `GameResultSystem` もここに入れるか検討
- [ ] Component フォルダ: 肥大化確認
- [ ] Data フォルダ: 全 IO クラスが揃っているか
- [ ] `FieldObject/` フォルダの構造が `Editor/` `System/` `Component/` に揃っているか

---

### 9-2. 整理実行

精査結果に基づいて実施:

- [ ] OutGame フォルダを `System/OutGame/` に確立し、ClearOverSystem/GameResultSystem を移動
- [ ] 不要になったファイル・空ファイルの削除
- [ ] `vcxproj` / `vcxproj.filters` を全ファイルの移動に追従させる
- [ ] include パスが全て正しいか確認 (ビルドエラーで検出)
- [ ] 旧 `RailData/` フォルダが空になっていれば削除

---

### Phase 9 完了確認

- [ ] ビルドエラーなし
- [ ] フォルダ構造が命名規則と一致している
- [ ] 不要ファイルがない
- [ ] ゲームが正常にプレイ可能

---

## 全体チェックリスト

| Phase | 項目 | 完了 |
|-------|------|------|
| 1 | ColliderConfig 汎用構造体作成 | [x] |
| 1 | Collider3D/2D にオフセット追加 | [x] |
| 1 | PlayerConfigComponent → PlayerConfig リネーム | [x] |
| 1 | EnemyTypePreset → EnemyConfig リネーム | [x] |
| 1 | ImGui + JSON 保存 (コライダー設定) | [x] |
| 2 | Orb → 吹き出し リネーム | [x] |
| 2 | SpeechBubbleConfig 作成 | [x] |
| 2 | カメラ距離による大中小判定 | [x] |
| 2 | ImGui + JSON 保存 (吹き出し設定) | [x] |
| 3 | Data/Config/ / Data/StageData/ フォルダ作成 | [ ] |
| 3 | 全 JSON ファイル移動 | [ ] |
| 3 | StageDataIO クラス作成 | [ ] |
| 3 | 全 IO クラスのパス更新 | [ ] |
| 4 | InputHelper 作成 | [ ] |
| 4 | PauseSystem / OptionSystem パッド対応 | [ ] |
| 4 | PlayerControlSystem スティック対応 | [ ] |
| 5 | FieldObjectEditor クラス作成 | [ ] |
| 5 | EditorManager 作成 | [ ] |
| 5 | エディタモード + エンジンポーズ連動 | [ ] |
| 5 | スポーン位置可視化 | [ ] |
| 6 | BossBehaviorSystem レールカメラ参照に変更 | [ ] |
| 6 | ボス初期スポーン (triggerDistance=0.5) | [ ] |
| 6 | GameResultComponent 作成 | [ ] |
| 7 | GameResultSystem 作成 | [ ] |
| 7 | BossDefeatSystem 作成 | [ ] |
| 7 | ClearOverStateComponent 作成 | [ ] |
| 7 | ClearOverSystem 作成 | [ ] |
| 7 | ClearOverViewSystem 作成 | [ ] |
| 7 | クリア/オーバー全フロー動作確認 | [ ] |
| 8 | 全音声ファイル登録 | [ ] |
| 8 | BGM 再生 (ゲーム/タイトル) | [ ] |
| 8 | 戦闘 SE 実装 | [ ] |
| 8 | 吹き出し SE 実装 | [ ] |
| 8 | システム SE 実装 | [ ] |
| 9 | フォルダ構造精査 | [ ] |
| 9 | 整理・不要ファイル削除 | [ ] |

---

## 注意事項・備忘録

### vcxproj 更新について
ファイル追加・削除・移動を行ったら必ず `NoEngine.vcxproj` と `NoEngine.vcxproj.filters` を更新する。
Visual Studio のソリューションエクスプローラーから追加・削除するのが最も確実。

### Phase 間の依存関係
```
Phase 1 → Phase 2, 3, 4 が依存
Phase 3 → Phase 5, 6 が依存 (ファイルパスの安定後)
Phase 6 → Phase 7 が依存 (GameResultComponent, ボス構造)
Phase 7 → Phase 8 の一部 (ClearOver SE) が依存
Phase 1〜8 完了 → Phase 9
```

### クリアロゴ・オーバーロゴの差し替え
`kClearLogo` / `kOverLogo` を TitleRogo.png の仮設定にしてある。
正式素材が用意できたら `GameResourceComponent.h` の `InitializeCommentBoutGameResources()` 内のパスを変更するだけで差し替え可能。

### 音量設定
`CBGameAudio::PlaySEClip()` / `PlayBGMClip()` の volume 引数は
`CBGameAudio::ApplyOptionVolumes()` で設定されたマスター・BGM・SE ボリュームが適用される。
ベース volume は 1.0f で統一し、OptionSystem の設定に委ねる。
