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
    };

    No::Vector2 gridSize_ = No::Vector2(64.f,64.f);
    No::Vector2 addRoomPosition_{};

    No::Vector2 mousePosition_{};
    
    No::Vector2 mouseOffset_{};


    EditState state_ = EditState::kRoom;

    void AddBlock(No::Registry& registry);
    void AddRoom(No::Registry& registry);
    void AddSave(No::Registry& registry);
    void DeleteGimmick(No::Registry& registry);
    void DeleteBlock(No::Registry& registry);
    No::Entity FindRoom(No::Registry& registry, const No::Vector2& pos);
    No::Vector2 GetGridPosition(const No::Vector2& position);

    void DrawEditWindow(No::Registry& registry);
    void DrawRooms(No::Registry& registry);
};

