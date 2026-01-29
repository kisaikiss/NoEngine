#pragma once
#include "engine/NoEngine.h"
#include"../EnemyStateManager.h"
#include "../../../Component/PhysicsComponent.h"
#include"../../../Component/NormalEnemyComponent.h"
#include<memory>
class NormalEnemyControlSystem : public No::ISystem
{
public:
    NormalEnemyControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
private:

};

template<typename EnemyComponent>
class EnemyAppear :public BaseEnemyState<EnemyComponent> {

public:
    EnemyAppear() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;
private:
    float timer_ = 0.0f;

};
class EnemyMove :public BaseEnemyState<NormalEnemyComponent> {
public:

    EnemyMove() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;
private:
    float theta_ = 0.0f;
};

template<typename EnemyComponent>
class EnemyHit :public BaseEnemyState<EnemyComponent> {
public:
    EnemyHit() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;
private:
    float timer_ = 0.0f;
    PhysicsComponent* ballPhysics_ = nullptr;
};

template<typename EnemyComponent>
class EnemyDie :public BaseEnemyState<EnemyComponent> {
public:
    EnemyDie() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;


private:
    float timer_ = 0.0f;

};