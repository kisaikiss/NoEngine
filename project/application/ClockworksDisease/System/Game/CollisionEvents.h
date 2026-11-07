#pragma once
#include "engine/NoEngine.h"

struct PlayerEnemyContactEvent {
	No::Entity player;
	No::Entity enemy;
};

struct PlayerAttackHitEvent {
	No::Entity player;
	No::Entity enemy;
};

struct PlayerPushBackEvent { 
	No::Entity player;
	No::Entity platform;
	No::ContactPosition position;
	No::Vector3 normal;
	float penetration = 0.0f;
};


struct ItemGetEvent {
	No::Entity player;
	No::Entity item;
};
