#include "../../Window.h"
#include "MainEditorWindowCloseEvent.h"


std::optional<LRESULT> NoEngine::MainEditorWindowCloseEvent::OnEvent(UINT /*msg*/, WPARAM /*wparam*/, LPARAM /*lparam*/) {
    HWND hwnd = GetWindow()->GetWindowHandle();
    // ウィンドウ終了確認ダイアログを表示
    int result = MessageBox(
        hwnd,
        L"終了しますか？",
        L"確認",
        MB_YESNO | MB_ICONQUESTION
    );
    if (result == IDYES) {
        GetWindow()->Destroy();
    }

    return 0;
}
