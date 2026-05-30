#pragma once
#include "engine/NoEngine.h"
class RabbitdokuScene :
    public No::IScene {
public:
    RabbitdokuScene(const std::string& musicName = "title", float bgmVolume = 1.f) : backgroundMusicName_(musicName),soundVolume_(bgmVolume){}
    
    void Setup() override;
protected:
    void InitGameScene();
    virtual void InitBackground(No::Registry& registry);
private:
    std::string backgroundMusicName_;
    float soundVolume_;

    void AddSystems();
    
    void InitPlayer(No::Registry& registry);
    void InitCamera(No::Registry& registry);
    void PlayMusic();

    void InitRoom(No::Registry& registry);

    void NotSystemUpdate() override;
};

