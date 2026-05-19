#include "stdafx.h"
#include "RabbitdokuOdyssey3Plus.h"
#include "Scene/RabbitdokuScene.h"

void RabbitdokuOdyssey3Plus::Startup(void) {
	RegisterScene("Rabbitdoku", []() { return std::make_unique<RabbitdokuScene>();	});
	ChangeScene("Rabbitdoku");

	auto* roomBox = NoEngine::ComponentRegistry::FindByName("RoomTag");
	roomBox->fields[0].attributes.valueSpeed = 1.f;
	roomBox->fields[1].attributes.valueSpeed = 1.f;
}
