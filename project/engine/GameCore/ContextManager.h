#pragma once

namespace NoEngine {
/// <summary>
/// 前方宣言
/// </summary>
class CommandContext;

/// <summary>
/// CommandContextを管理、再利用するためのクラス
/// </summary>
class ContextManager {
public:
    ContextManager(void);
    ~ContextManager();

    /// <summary>
    /// 指定した Direct3D 12 のコマンドリスト種別用の CommandContext を割り当てます。
    /// </summary>
    /// <param name="type">D3D12_COMMAND_LIST_TYPE で指定するコマンドリストの種類。</param>
    /// <returns>割り当てられた CommandContext へのポインタ。割り当てに失敗した場合は nullptr を返すことがあります。</returns>
    CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE type);

    /// <summary>
    /// 使用済みのCommandContextをプールに戻します。
    /// </summary>
    /// <param name="">使用済みCommandContext</param>
    void FreeContext(CommandContext*);

    /// <summary>
    /// すべてのコンテキストを破棄します。
    /// </summary>
    void DestroyAllContexts();

private:
    static inline const uint32_t kContextNum = 4;
    std::vector<std::unique_ptr<CommandContext> > contextPool_[kContextNum];
    std::queue<CommandContext*> availableContexts_[kContextNum];
    std::mutex contextAllocationMutex_;
};

}
