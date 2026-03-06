#include "engine/Runtime/GameCore.h"
#include "application/TD3/OneSide.h"
#include "application/TestApp/TestApp.h"
#include "application/TD2_3/Game.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return NoEngine::GameCore::RunApplication(std::make_unique<OneSide>()); }