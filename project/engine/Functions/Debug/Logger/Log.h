#pragma once

namespace NoEngine {
enum class VerbosityLevel {
    kCritical,
    kError,
    kWarning,
    kInfo,
    kDebug
};

// ImGui表示用にログの1行を管理する構造体
struct LogEntry {
    VerbosityLevel level;
    std::string timeStr;
    std::string message;
};

class Log {
public:
    // Singletonインスタンスの取得
    static Log& GetInstance() {
        static Log instance;
        return instance;
    }

    void Initialize();
    void SetVerbosityLevel(VerbosityLevel verbosityLevel);
    void Print(VerbosityLevel level, const std::string& message);

    // ImGui描画用関数
    void DrawImGuiWindow(const char* title, bool* p_open = nullptr);

private:
    Log() = default;
    ~Log();

    // コピー禁止
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    VerbosityLevel m_VerbosityLevel = VerbosityLevel::kDebug;
    std::string m_FilePath;
    std::ofstream m_LogStream;

    // スレッドセーフ用のミューテックス
    std::mutex m_Mutex;

    // ImGui表示用のログ履歴（メモリ肥大化を防ぐため上限を設ける）
    std::deque<LogEntry> m_LogEntries;
    const size_t MAX_LOG_ENTRIES = 1000;

    // ImGuiの自動スクロールフラグ
    bool m_AutoScroll = true;
};
}


void LogCritical(const std::string& message);
void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogDebug(const std::string& message);