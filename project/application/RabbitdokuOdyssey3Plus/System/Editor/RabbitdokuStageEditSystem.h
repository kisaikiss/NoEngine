#pragma once
#include "engine/NoEngine.h"
class RabbitdokuStageEditSystem :
    public No::ISystem {
public:
    RabbitdokuStageEditSystem() { SetStopInGameStop(false); SetStopInGameStop(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
    enum class EditState {
        kBlock,
        kRoom,
        kGimmick,
        kBackground,
        kPlayer
    };

    enum class GimmickSelected {
        kSave,
        kDeathBlock,
        kNeedle,
        kSpring,
        kCollapseBlock,
        kReplenisher,
        kDoor,
        kGoal,
    };

    struct BlockTextures {
        static inline std::string kDaionkai = "resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png";
        static inline std::string kBlue = "resources/game/RabbitdokuOdyssey3Plus/Sprite/block02.png";
        static inline std::string kEx = "resources/game/RabbitdokuOdyssey3Plus/Sprite/exBlock.png";
    };

    struct BackgroundTextures {
        static inline std::string kFlower = "resources/game/RabbitdokuOdyssey3Plus/Sprite/flower.png";
        static inline std::string kTitle = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Title/GameTitle.png";
        static inline std::string kBackground01 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject01.png";
        static inline std::string kBackground02 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject02.png";
        static inline std::string kBackground03 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject03.png";
        static inline std::string kBackground04 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject04.png";
        static inline std::string kBackground05 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject05.png";
        static inline std::string kBackground06 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject06.png";
        static inline std::string kBackground07 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject07.png";
        static inline std::string kBackground08 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject08.png";
        static inline std::string kBackground09 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/DirectionObject09.png";

        static inline std::string kStageNumber00 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber00.png";
        static inline std::string kStageNumber01 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber01.png";
        static inline std::string kStageNumber02 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber02.png";
        static inline std::string kStageNumber03 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber03.png";
        static inline std::string kStageNumber04 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber04.png";
        static inline std::string kStageNumber05 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber05.png";
        static inline std::string kStageNumber06 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber06.png";
        static inline std::string kStageNumber07 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber07.png";
        static inline std::string kStageNumber08 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber08.png";
        static inline std::string kStageNumber09 = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Background/StageNumber09.png";
    };

    No::Vector2 gridSize_ = No::Vector2(64.f,64.f);
    No::Vector2 addRoomPosition_{};

    No::Vector2 mousePosition_{};
    
    No::Vector2 mouseOffset_{};

    std::string currentBlockTexture = BlockTextures::kBlue;
    std::string currentBackgroundTexture_ = BackgroundTextures::kTitle;
    EditState state_ = EditState::kBlock;
    GimmickSelected gimmick_ = GimmickSelected::kDeathBlock;

    void AddBlock(No::Registry& registry);
    void AddRoom(No::Registry& registry);
    void AddSave(No::Registry& registry);
    void AddDeathBlock(No::Registry& registry);
    void AddNeedle(No::Registry& registry);
    void AddSpring(No::Registry& registry);
    void AddCollapseBlock(No::Registry& registry);
    void AddReplenisher(No::Registry& registry);
    void AddDoor(No::Registry& registry);
    void AddBackground(No::Registry& registry);
    void AddGoal(No::Registry& registry);
    void DeleteGimmick(No::Registry& registry);
    void DeleteBlock(No::Registry& registry);
    void WarpPlayer(No::Registry& registry);
    No::Entity FindRoom(No::Registry& registry, const No::Vector2& pos);
    No::Entity FindBackground(No::Registry& registry, const No::Vector2& pos);
    No::Vector2 GetGridPosition(const No::Vector2& position);

    void DrawEditWindow(No::Registry& registry);
    void DrawRooms(No::Registry& registry);
    void DrawCollider(No::Registry& registry);
};

