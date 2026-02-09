#include "StatusSpriteControlSystem.h"
#include"../SpriteConfigManager/SpriteConfigManager.h"
#include "../Component/PlayerstatusComponent.h"
#include "application/TD2_3/tag.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace
{
    No::Vector2 kBaseLevelFontPosition = { 200.0f,144.0f };
    const No::Vector2 kLevelFontScale = { 64.f, 64.f };
    const float kDigitUvWidth = 0.1f;
}

void StatusSpriteControlSystem::Update(No::Registry& registry, float deltaTime)
{
    (void)deltaTime;

    // プレイヤーステータスからレベル取得（存在しなければ0）
    auto PlayerStatusView = registry.View<PlayerStatusComponent>();
    int32_t level = 0;
    for (auto entity : PlayerStatusView)
    {
        auto* status = registry.GetComponent<PlayerStatusComponent>(entity);
        level = status->level;
    }

    // レベル表示は最大2桁。99を上限にする
    if (level > 99) level = 99;
    if (level < 0) level = 0;

    // 全ての Sprite/Transform を走査して "LevelFont" を集める
    auto spriteView = registry.View<No::Transform2DComponent, No::SpriteComponent>();
    std::vector<No::Entity> levelFontEntities;
    for (auto entity : spriteView)
    {
        auto* sp = registry.GetComponent<No::SpriteComponent>(entity);
        if (sp && sp->name == "LevelFont")
        {
            levelFontEntities.push_back(entity);
        }
    }

    // Helper: 保持している LevelFont の参照（あれば最初のをテンプレに使う）
    No::Entity templateEntity = 0;
    if (!levelFontEntities.empty()) templateEntity = levelFontEntities.front();

    // 必要な桁数を決定（1桁 or 2桁）
    const int neededDigits = (level >= 10) ? 2 : 1;

    // 既存の LevelFont が少なければ追加、多ければ削除（1つは残す）
    while ((int)levelFontEntities.size() < neededDigits)
    {
        No::Entity e = registry.GenerateEntity();
        auto* t = registry.AddComponent<No::Transform2DComponent>(e);
        auto* s = registry.AddComponent<No::SpriteComponent>(e);

        // 初期設定はテンプレがあればそれをコピー、なければ既定値をセット
        if (templateEntity != 0 && registry.Has<No::SpriteComponent>(templateEntity))
        {
            auto* templSp = registry.GetComponent<No::SpriteComponent>(templateEntity);
            *s = *templSp;
        }
        else
        {
            s->name = "LevelFont";
            s->uv.width = kDigitUvWidth;
            s->layer = 2;
            s->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/levelNumbers.png");
        }

        // Transform の既定値
        t->scale = kLevelFontScale;
        // 位置は後で一括設定するためここでは基準に合わせる
        levelFontEntities.push_back(e);

        // テンプレが未設定ならこの新規をテンプレにする
        if (templateEntity == 0) templateEntity = e;
    }

    while ((int)levelFontEntities.size() > neededDigits)
    {
        No::Entity last = levelFontEntities.back();
        levelFontEntities.pop_back();
        registry.DestroyEntity(last);
    }

    // 右（基準位置）を ones の位置とする。十の位があれば左に並べる。
    for (size_t i = 0; i < levelFontEntities.size(); ++i)
    {
        No::Entity e = levelFontEntities[i];
        auto* sp = registry.GetComponent<No::SpriteComponent>(e);
        auto* t = registry.GetComponent<No::Transform2DComponent>(e);
        if (!sp || !t) continue;

        const int idxFromRight = static_cast<int>(levelFontEntities.size() - 1 - i);
        if (idxFromRight == 0)
        {
            int digit = level % 10;
            // ones
            if (levelFontEntities.size() <= 1)
            {
                sp->uv.x = kDigitUvWidth * static_cast<float>(digit);
                t->translate = kBaseLevelFontPosition;
            }
            else
            {
                sp->uv.x = kDigitUvWidth * static_cast<float>(digit);
                No::Vector2 offset = { kLevelFontScale.x, 0.f };
                t->translate = kBaseLevelFontPosition + offset;
            }
        }
        else if (idxFromRight == 1)
        {
            int digit = (level / 10) % 10;
            sp->uv.x = kDigitUvWidth * static_cast<float>(digit);
            No::Vector2 offset = { kLevelFontScale.x, 0.f };
            t->translate = kBaseLevelFontPosition;
        }
        else
        {
            int digit = 0;
            if (idxFromRight == 2) digit = (level / 100) % 10;
            sp->uv.x = kDigitUvWidth * static_cast<float>(digit);
            No::Vector2 offset = { kLevelFontScale.x * static_cast<float>(idxFromRight), 0.f };
            t->translate = kBaseLevelFontPosition;
        }

        // サイズが異なる可能性があるため scale を再設定
        t->scale = kLevelFontScale;
        sp->uv.width = kDigitUvWidth;
        sp->name = "LevelFont";
    }

}