#include "stdafx.h"
#include "RabbitdokuOdyssey3Plus.h"
#include "Scene/RabbitdokuScene.h"

void RabbitdokuOdyssey3Plus::Startup(void) {
	RegisterScene("Rabbitdoku", []() { return std::make_unique<RabbitdokuScene>();	});
	ChangeScene("Rabbitdoku");
}
