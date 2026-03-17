#include "engine/Runtime/GameCore.h"
#include "application/TD3/OneSide.h"
#include "application/TestApp/TestApp.h"
#include "application/TD2_3/Game.h"
#include "application/CommentBout/CommentBout.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return NoEngine::GameCore::RunApplication(std::make_unique<CommentBout>()); }