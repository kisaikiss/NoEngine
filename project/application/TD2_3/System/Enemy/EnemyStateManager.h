#pragma once
#include"BaseEnemyState.h"
#include<shared_mutex>
#include <memory> 
#include <utility> 
#include <tuple> 
#include <functional>
template<typename EnemyOwnerType>

class EnemyStateManager {
public:

    EnemyStateManager() :fnChangeState_([]() {}) {};
    //EnemyStateManager() = default;
    EnemyStateManager(const EnemyStateManager&) = delete;
    EnemyStateManager& operator=(const EnemyStateManager&) = delete;

    void Start(EnemyOwnerType* ownerType) {
        owner_ = ownerType;
        fnChangeState_ = []() {};
    }

    //状態を変更する関数
    //ステートの変更処理を関数ポインタの中に閉じ込め更新が終わった後に呼ぶ
    //ステートの型を指定したら関数内でインスタンスを作成し、コンストラクタに値を渡せるように変更
    template<typename StateType, typename ...ArgType>
    void ChangeState(No::Registry& registry, ArgType&&... args) {
        // 引数をタプルにまとめてキャプチャ 
        auto argsTuple = std::make_tuple(std::forward<ArgType>(args)...);
        //ステートの変更命令を格納する
        //ステートの変更命令を関数ポインタに格納する
        fnChangeState_ = [this, &registry, argsTuple = std::move(argsTuple)]() mutable {


            if (owner_ == nullptr)   return;

            if (state_ != nullptr) {
                state_->CallExit(registry, owner_);
                state_ = nullptr;
            }
            // 新しいステートを作成
            state_ = std::apply([](auto&&... unpackedArgs) { 
                return std::make_shared<StateType>(std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
                },
                std::move(argsTuple)
            );

            if (state_ == nullptr) { 
                std::puts("Failed to create state!");
                return;
            }

            //新しいステートにこのマシーンをセット
            state_->SetEnemyStateManager(this);
            state_->CallEnter(registry, owner_);
            
            };
    }

    void Update(No::Registry& registry,float deltaTime) {

        fnChangeState_();
        fnChangeState_ = []() {};

        if (state_ != nullptr) {
            state_->CallUpdate(registry, owner_,deltaTime);
        }
    }

private:

    //現在の持ち主のポインタ
    EnemyOwnerType* owner_ = nullptr;
    //今のステート
    std::shared_ptr<BaseEnemyState<EnemyOwnerType>> state_ = nullptr;
    //ステート変更命令を保存しておく関数オブジェクト
    std::function<void()>fnChangeState_;
};