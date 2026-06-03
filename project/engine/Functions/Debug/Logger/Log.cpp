#include "Log.h"
#include <dxgidebug.h>
#include <filesystem>
#include <fstream>
#include <chrono>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
Log::~Log() {
    if (m_LogStream.is_open()) {
        m_LogStream.close();
    }
}

void Log::Initialize() {
    std::filesystem::create_directory("logs");

    std::vector<std::filesystem::directory_entry> logFiles;
    for (const auto& entry : std::filesystem::directory_iterator("logs")) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            logFiles.push_back(entry);
        }
    }

    std::sort(logFiles.begin(), logFiles.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });

    const size_t maxLogFiles = 100;
    if (logFiles.size() > maxLogFiles) {
        for (size_t i = 0; i < logFiles.size() - maxLogFiles; ++i) {
            std::filesystem::remove(logFiles[i]);
        }
    }

    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
    m_FilePath = "logs/" + dateString + ".log";

    // ここでストリームを開きっぱなしにする
    m_LogStream.open(m_FilePath, std::ios::out | std::ios::app);
}

void Log::SetVerbosityLevel(VerbosityLevel verbosityLevel) {
    m_VerbosityLevel = verbosityLevel;
}

void Log::Print(VerbosityLevel level, const std::string& message) {
    if (m_VerbosityLevel < level) return; // 設定レベルより詳細なログは弾く

    // ログ出力時間の取得
    auto now = std::chrono::system_clock::now();
    std::string timeStr = std::format("{:%H:%M:%S}", std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::time_point_cast<std::chrono::seconds>(now) });

    std::string prefix;
    switch (level) {
    case VerbosityLevel::kCritical: prefix = "[CRITICAL] "; break;
    case VerbosityLevel::kError:    prefix = "[ERROR]    "; break;
    case VerbosityLevel::kWarning:  prefix = "[WARNING]  "; break;
    case VerbosityLevel::kInfo:     prefix = "[INFO]     "; break;
    case VerbosityLevel::kDebug:    prefix = "[DEBUG]    "; break;
    }

    std::string formattedMessage = std::format("[{}] {}{}", timeStr, prefix, message);

    // --- マルチスレッド保護区間 ---
    std::lock_guard<std::mutex> lock(m_Mutex);

    // 1. ファイルへの出力
    if (m_LogStream.is_open()) {
        m_LogStream << formattedMessage << "\n";
        // エラー以上の場合は即座にフラッシュして書き込みを保証する
        if (level <= VerbosityLevel::kError) {
            m_LogStream.flush();
        }
    }

    // 2. Visual Studio出力ウィンドウへの出力
    OutputDebugStringA((formattedMessage + "\n").c_str());

    // 3. ImGui用のメモリ保存
    m_LogEntries.push_back({ level, timeStr, message });
    if (m_LogEntries.size() > MAX_LOG_ENTRIES) {
        m_LogEntries.pop_front(); // 上限を超えたら古いものから削除
    }
}

void Log::DrawImGuiWindow(const char* title, bool* p_open) {
#ifdef USE_IMGUI

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    // オプション配置（クリアボタン、オートスクロール）
    if (ImGui::Button("Clear")) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_LogEntries.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
    ImGui::Separator();

    // ログ表示領域
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    {
        // 描画中は新しいログが追加されないようにロック
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (const auto& entry : m_LogEntries) {
            // レベルに応じてテキストの色を変える
            ImVec4 color;
            bool hasColor = true;
            switch (entry.level) {
            case VerbosityLevel::kCritical: color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break; // 赤
            case VerbosityLevel::kError:    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break; // 薄い赤
            case VerbosityLevel::kWarning:  color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break; // 黄色
            case VerbosityLevel::kDebug:    color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); break; // グレー
            default: hasColor = false; break; // デフォルト色(Info)
            }

            if (hasColor) ImGui::PushStyleColor(ImGuiCol_Text, color);

            // プレフィックス(時間など)とメッセージを表示
            ImGui::TextUnformatted(std::format("[{}] {}", entry.timeStr, entry.message).c_str());

            if (hasColor) ImGui::PopStyleColor();
        }
    }

    // 一番下までスクロール
    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
#else 
    static_cast<void>(title);
    static_cast<void>(p_open);
#endif // USE_IMGUI

}

}


void LogCritical(const std::string& message) { NoEngine::Log::GetInstance().Print(NoEngine::VerbosityLevel::kCritical, message); }
void LogError(const std::string& message) { NoEngine::Log::GetInstance().Print(NoEngine::VerbosityLevel::kError, message); }
void LogWarning(const std::string& message) { NoEngine::Log::GetInstance().Print(NoEngine::VerbosityLevel::kWarning, message); }
void LogInfo(const std::string& message) { NoEngine::Log::GetInstance().Print(NoEngine::VerbosityLevel::kInfo, message); }
void LogDebug(const std::string& message) { NoEngine::Log::GetInstance().Print(NoEngine::VerbosityLevel::kDebug, message); }