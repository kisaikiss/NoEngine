#pragma once
#include "engine/NoEngine.h"

struct PlayerPushBackEvent { 
	No::Entity player;
	No::ContactPosition position;
};