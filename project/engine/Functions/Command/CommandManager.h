#pragma once

namespace NoEngine {
namespace Command {
class ICommand {
	friend class CommandManager;
public:
	ICommand(const std::string& commandName = "UnnamedCommand") : name_(commandName){}
	virtual ~ICommand() = default;
	virtual void Execute() = 0;
	virtual void Undo() = 0;
private:
	std::string name_;
};

class CommandManager {
public:
	~CommandManager() {
		undoStack_.clear();
		redoStack_.clear();
	}

	/// <summary>
	/// Undoスタックへコマンドの追加を行います
	/// </summary>
	/// <param name="cmd">追加するコマンド</param>
	void AddCommand(std::unique_ptr<ICommand> cmd);

	/// <summary>
	/// Undoスタックのコマンドを取り出し、Redoスタックへ移動します。
	/// </summary>
	void Undo();

	/// <summary>
	/// Redoスタックのコマンドを取り出し、Undoスタックへ移動します。
	/// </summary>
	void Redo();

private:
	std::deque<std::unique_ptr<ICommand>> undoStack_;
	std::deque<std::unique_ptr<ICommand>> redoStack_;
};
}
}