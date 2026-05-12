#pragma once
#include "engine/NoEngine.h"
class RabbitdokuScene :
    public No::IScene {
public:
    void Setup() override;
private:
    void AddSystems();
    
    void InitPlayer(No::Registry& registry);
    void InitCamera(No::Registry& registry);

    void InitBlock(No::Registry& registry);
    void InitRoom(No::Registry& registry);
};

