#pragma once
#include "engine/NoEngine.h"
#include"EnemyStateManager.h"

class NormalEnemyControlSystem : public No::ISystem
{
public:
    NormalEnemyControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
private:
    //ステートマネージャー
    EnemyStateManager<NormalEnemyControlSystem> enemyStateManager_;
    float timer_ = 0.0f;
};


class EnemyAppear :public BaseEnemyState<NormalEnemyControlSystem> {

public:
    EnemyAppear() = default;
    //ステートが始まるときに一度だけ呼ばれる
     void Enter(No::Registry& registry,NormalEnemyControlSystem* ownerType) override;
     //ステートが更新時に呼ばれる
     void Update(No::Registry& registry, NormalEnemyControlSystem* ownerType,float deltaTime)override;
     //ステートが終了するときに一度だけ呼ばれる
     void Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)override;
};

class EnemyChase :public BaseEnemyState<NormalEnemyControlSystem> {
public:

EnemyChase() = default; 
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry, NormalEnemyControlSystem* ownerType) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, NormalEnemyControlSystem* ownerType,float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)override;



};

class EnemyHit :public BaseEnemyState<NormalEnemyControlSystem> {
public:
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry, NormalEnemyControlSystem* ownerType) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, NormalEnemyControlSystem* ownerType)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)override;
};

class EnemyInvincible :public BaseEnemyState<NormalEnemyControlSystem> {



};

class EnemyDie :public BaseEnemyState<NormalEnemyControlSystem> {



};

