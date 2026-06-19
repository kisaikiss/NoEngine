#pragma once
#include "engine/NoEngine.h"

struct PlayerPushBackEvent { 
	No::Entity player;
	No::ContactPosition position;
	No::Vector3 normal;
};


struct ItemGetEvent {
	No::Entity player;
	No::Entity item;
};