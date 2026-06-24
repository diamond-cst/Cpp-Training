#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include <string>
#include <vector>

// 回放记录管理器
class ReplayManager {
private:
    std::string directory;  // 存放回放记录的目录
    std::string textFilename;   // 当前回放的文本记录文件名
    std::string htmlFilename;   // 当前回放的HTML记录文件名
    std::vector<std::string> lines; // 存储回放记录的文本行
    bool active;          // 是否正在记录回放

public:
    explicit ReplayManager(const std::string& dir = "replays");

    // 开始记录回放，传入玩家名字、魔术类型和模式名称，用于生成文件名和记录基本信息
    void start(const std::string& playerName,
               const std::string& trickName,
               const std::string& modeName);
    // 记录每轮的牌堆状态和观众选择
    void recordRound(int round,
                     const std::string& pile1,
                     const std::string& pile2,
                     const std::string& pile3,
                     int chosenPile);
    // 记录揭示牌的结果
    void recordReveal(const std::string& card, bool correct, int finalScore);
    // 保存回放记录到文本文件
    void save() const;
    // 导出回放记录为HTML文件
    void exportHtml() const;
    bool isActive() const;
    std::string getTextFilename() const;
    std::string getHtmlFilename() const;

    // 列出回放目录下的所有回放记录文件，返回文件名列表
    static std::vector<std::string> listReplayFiles(const std::string& dir = "replays");
    // 显示指定回放记录文件的内容，供用户查看
    static void displayReplayFile(const std::string& filename);

private:
    // 确保回放目录存在，如果不存在则创建
    void ensureDirectory() const;
    // 文件名安全化，去除不合法字符
    static std::string sanitizeFilePart(const std::string& value);
    // 转义HTML特殊字符，生成安全的HTML内容
    static std::string escapeHtml(const std::string& value);
};

#endif // REPLAYMANAGER_H
