#pragma once
#include "engine/NoEngine.h"

template<typename>
class EnemyStateManager;

template<typename EnemyOwnerType>
class BaseEnemyState
{
protected:
    //ステートマネージャー以外から呼び出し関数などにアクセスできないように
    friend class EnemyStateManager<EnemyOwnerType>;
    BaseEnemyState() = default;
    virtual  ~BaseEnemyState() = default;
    //ステートが始まるときに一度だけ呼ばれる
    virtual void Enter(No::Registry& registry) = 0;
    //ステートの更新時に呼ばれる
    virtual void Update(No::Registry& registry,float deltaTime) = 0;
    //ステートが終了するときに一度だけ呼ばれる
    virtual void Exit(No::Registry& registry) = 0;

private:
    // この状態を管理しているステートマシーンをセット
    void SetEnemyStateManager(EnemyStateManager<EnemyOwnerType>* stateManager) {
        stateManager_ = stateManager;
    }

    // 開始関数をマネージャーから呼ぶための関数
    void CallEnter(No::Registry& registry, EnemyOwnerType* ownerType) {
    
        ownerType_ = ownerType;
        if (stateManager_ == nullptr || ownerType_ == nullptr) {
            return;
        }
    
        Enter(registry);
    }
    // 更新関数をマネージャーから呼ぶための関数
    void CallUpdate(No::Registry& registry, EnemyOwnerType* ownerType,float deltaTime) {
        ownerType_ = ownerType;
        if (stateManager_ == nullptr || ownerType_ == nullptr) {
            return;
        }

        Update(registry, deltaTime);
    }

    // 終了関数をマネージャーから呼ぶための関数
    void CallExit(No::Registry& registry, EnemyOwnerType* ownerType) {
        ownerType_ = ownerType;
        if (stateManager_ == nullptr || ownerType_ == nullptr) {
            return;
        }

        Exit(registry);
    }
protected:
    //このステートマネージャーのポインタを保存
    EnemyStateManager<EnemyOwnerType>* stateManager_ = nullptr;
    EnemyOwnerType* ownerType_ = nullptr;
};

