#pragma once
#include "engine/NoEngine.h"
#include"EnemyStateManager.h"
#include "../../Component/PhysicsComponent.h"
#include"../../Component/NormalEnemyComponent.h"
#include<memory>
class NormalEnemyControlSystem : public No::ISystem
{
public:
    NormalEnemyControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
private:
    bool isApper_ = false;
    std::unique_ptr< EnemyStateManager<NormalEnemyComponent>>stateManager_ = nullptr;
};


class EnemyAppear :public BaseEnemyState<NormalEnemyComponent> {

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
class EnemyChase :public BaseEnemyState<NormalEnemyComponent> {
public:

    EnemyChase() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;

};

class EnemyHit :public BaseEnemyState<NormalEnemyComponent> {
public:
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

class EnemyInvincible :public BaseEnemyState<NormalEnemyComponent> {



private:
    float timer_ = 0.0f;
};


class EnemyDie :public BaseEnemyState<NormalEnemyComponent> {
public:
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;


private:
    float timer_ = 0.0f;

};

/// @brief PoyoPyoアニメーション
/// @param transform 位置
/// @param timer アニメーションタイマー
/// @param speed PoyoPoyo周期
/// @param defaultScale デフォルトの大きさ
void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const NoEngine::Vector3& defaultScale = NoEngine::Vector3::UNIT_SCALE);

void TimerUpdate(float& timer, float& deltaTime);

void LookTarget(No::TransformComponent& transform, const NoEngine::Vector3& target);

NoEngine::Vector3 GatTargetDir(NoEngine::Vector3& translate, const NoEngine::Vector3& target);

/// @brief イーズインアウトバック
/// @param t 時間
/// @return イージングされた時間
float EaseInOutBackT(const float& t);
NoEngine::Vector3 EaseInOutBack(const NoEngine::Vector3& start, const NoEngine::Vector3& end, float t);