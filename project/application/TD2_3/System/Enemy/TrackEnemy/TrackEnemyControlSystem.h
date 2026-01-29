#pragma once
#include "engine/NoEngine.h"
#include"../EnemyStateManager.h"
#include "../../../Component/PhysicsComponent.h"
#include"../../../Component/TrackEnemyComponent.h"
#include<memory>

class TrackEnemyControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
}; 

class EnemyChase :public BaseEnemyState<TrackEnemyComponent> {
public:

    EnemyChase() = default;
    //ステートが始まるときに一度だけ呼ばれる
    void Enter(No::Registry& registry) override;
    //ステートが更新時に呼ばれる
    void Update(No::Registry& registry, float deltaTime)override;
    //ステートが終了するときに一度だけ呼ばれる
    void Exit(No::Registry& registry)override;

};
