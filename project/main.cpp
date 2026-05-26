#include "engine/Runtime/GameCore.h"
#include "application/RabbitdokuOdyssey3Plus/RabbitdokuOdyssey3Plus.h"
#include "application/ClockworksDisease/ClockworksDisease.h"
#include "application/TestApp/TestApp.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return NoEngine::GameCore::RunApplication(std::make_unique<RabbitdokuOdyssey3Plus>()); }