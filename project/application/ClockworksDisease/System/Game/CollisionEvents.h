#pragma once
#include "engine/NoEngine.h"

struct PlayerPushBackEvent { 
	No::Entity player;
	No::ContactPosition position;
	No::Vector3 normal;
	float penetration = 0.0f;
};


struct ItemGetEvent {
	No::Entity player;
	No::Entity item;
};