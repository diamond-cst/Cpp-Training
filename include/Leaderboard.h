#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string>
#include <vector>
#include <algorithm>

// 玩家记录结构
struct PlayerRecord {
    std::string name;       // 玩家名称
    int score;              // 分数
    int gamesPlayed;        // 游戏次数
    int correctGuesses;     // 正确次数
    int streak;             // 连胜次数
    std::string timestamp;  // 时间戳

    // 构造函数
    PlayerRecord();
    PlayerRecord(const std::string& n, int s, int gp, int cg, int st, const std::string& ts);

    // 比较运算符（用于排序）
    bool operator<(const PlayerRecord& other) const;
    bool operator>(const PlayerRecord& other) const;
};

// 排行榜类
class Leaderboard {
private:
    std::vector<PlayerRecord> records;  // 玩家记录列表
    std::string filename;               // 存储文件名
    static const int MAX_RECORDS = 10;  // 最多保存10条记录

public:
    // 构造函数
    explicit Leaderboard(const std::string& file = "leaderboard.dat");

    // 析构函数
    ~Leaderboard();

    // 添加或更新玩家记录
    void addOrUpdateRecord(const PlayerRecord& record);

    // 获取排行榜
    std::vector<PlayerRecord> getTopRecords(int count = MAX_RECORDS) const;

    // 获取玩家记录
    PlayerRecord* getPlayerRecord(const std::string& playerName);

    // 显示排行榜
    void display() const;

    // 显示带颜色的排行榜
    void displayColored() const;

    // 保存到文件
    void save() const;

    // 从文件加载
    void load();

    // 清空排行榜
    void clear();

    // 获取记录数量
    int getRecordCount() const;

    // 检查玩家是否在排行榜中
    bool hasPlayer(const std::string& playerName) const;

    // 获取玩家排名
    int getPlayerRank(const std::string& playerName) const;

private:
    // 排序记录（按分数降序）
    void sortRecords();

    // 限制记录数量
    void trimRecords();
};

#endif // LEADERBOARD_H
