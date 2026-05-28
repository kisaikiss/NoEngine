#pragma once
#include "engine/NoEngine.h"
class RabbitdokuScene :
    public No::IScene {
public:
    void Setup() override;
protected:
    void InitGameScene();
    virtual void InitBackground(No::Registry& registry);
private:
    void AddSystems();
    
    void InitPlayer(No::Registry& registry);
    void InitCamera(No::Registry& registry);

    void InitRoom(No::Registry& registry);

    void NotSystemUpdate() override;
};

