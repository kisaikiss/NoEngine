#pragma once

namespace NoEngine {

/// <summary>
/// このクラスを引数リストへ入れると指定したTからにのみアクセスを許す
/// </summary>
/// <typeparam name="T">Tからのみアクセスを許す</typeparam>
template <typename T>
class Passkey final {
    friend T;
    Passkey() = default;
};

/// <summary>
/// これを引数リストに入れるとWinMainからのみアクセスを許す
/// </summary>
class AllowAccessOnlyFromWinMain final {
    friend int ::WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
    AllowAccessOnlyFromWinMain() = default;
};
}