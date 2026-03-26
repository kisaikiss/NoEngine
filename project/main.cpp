#include "engine/Runtime/GameCore.h"
#include "application/TestApp/TestApp.h"
#include "application/CommentBout/CommentBout.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return NoEngine::GameCore::RunApplication(std::make_unique<CommentBout>()); }