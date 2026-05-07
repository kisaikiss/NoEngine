#include "engine/Runtime/GameCore.h"
#include "application/TD3/Celetste.h"
#include "application/ClockworksDisease/ClockworksDisease.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return NoEngine::GameCore::RunApplication(std::make_unique<Celetste>()); }