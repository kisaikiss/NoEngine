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

    VerbosityLevel verbosityLevel_ = VerbosityLevel::kDebug;
    std::string filePath_;
    std::ofstream logStream_;

    // スレッドセーフ用のミューテックス
    std::mutex mutex_;

    // ImGui表示用のログ履歴（メモリ肥大化を防ぐため上限を設ける）
    std::deque<LogEntry> logEntries;
    const size_t kMaxLogEntries = 1000;

    // ImGuiの自動スクロールフラグ
    bool autoScroll_ = true;
};
}


void LogCritical(const std::string& message);
void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogDebug(const std::string& message);