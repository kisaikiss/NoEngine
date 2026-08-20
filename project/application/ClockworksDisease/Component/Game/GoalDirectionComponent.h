#pragma once
#include "engine/NoEngine.h"

// ゴール到達時に、演出用カメラ・ゴールオブジェクトのTransformRoutineComponent再生完了を待ってから
// SceneChangeEventを発行し、GameClearSceneへフェード遷移するための管理コンポーネント。
// ItemGetSystemがゴールタグ付きアイテムとの接触を検知した瞬間に、この管理コンポーネントを持つ
// 専用Entityを1つ生成することで演出を開始する(実際の進行はGoalDirectionSystemが担当する)。
struct GoalDirectionComponent {
	No::Entity directorCamera = No::INVALID_ENTITY; // 演出用カメラ(シーンにあらかじめ配置しておく)
	No::Entity goalEntity = No::INVALID_ENTITY;     // 動かすゴールオブジェクト(演出用TransformRoutineComponentを持たせておく)
	No::Entity player = No::INVALID_ENTITY;         // 演出中、操作をロックする対象
};

// 演出中、プレイヤーの移動/ジャンプ/重力系Systemに処理をスキップさせるためのタグ。
struct GoalDirectionLockTag {};

// 演出用カメラを検索するために付けておくタグ
struct GoalDirectorCameraTag{};