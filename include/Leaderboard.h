#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string>
#include <vector>
#include <algorithm>

// 玩家记录结构 (Player record structure)
struct PlayerRecord {
    std::string name;       // 玩家名称 (Player name)
    int score;              // 分数 (Score)
    int gamesPlayed;        // 游戏次数 (Games played)
    int correctGuesses;     // 正确次数 (Correct guesses)
    int streak;             // 连胜次数 (Win streak)
    std::string timestamp;  // 时间戳 (Timestamp)

    // 构造函数 (Constructor)
    PlayerRecord();
    PlayerRecord(const std::string& n, int s, int gp, int cg, int st, const std::string& ts);

    // 比较运算符（用于排序）(Comparison operator for sorting)
    bool operator<(const PlayerRecord& other) const;
    bool operator>(const PlayerRecord& other) const;
};

// 排行榜类 (Leaderboard class)
class Leaderboard {
private:
    std::vector<PlayerRecord> records;  // 玩家记录列表 (Player records list)
    std::string filename;               // 存储文件名 (Storage filename)
    static const int MAX_RECORDS = 10;  // 最多保存10条记录 (Max 10 records)

public:
    // 构造函数 (Constructor)
    explicit Leaderboard(const std::string& file = "leaderboard.dat");

    // 析构函数 (Destructor)
    ~Leaderboard();

    // 添加或更新玩家记录 (Add or update player record)
    void addOrUpdateRecord(const PlayerRecord& record);

    // 获取排行榜 (Get leaderboard)
    std::vector<PlayerRecord> getTopRecords(int count = MAX_RECORDS) const;

    // 获取玩家记录 (Get player record)
    PlayerRecord* getPlayerRecord(const std::string& playerName);

    // 显示排行榜 (Display leaderboard)
    void display() const;

    // 显示带颜色的排行榜 (Display colored leaderboard)
    void displayColored() const;

    // 保存到文件 (Save to file)
    void save() const;

    // 从文件加载 (Load from file)
    void load();

    // 清空排行榜 (Clear leaderboard)
    void clear();

    // 获取记录数量 (Get record count)
    int getRecordCount() const;

    // 检查玩家是否在排行榜中 (Check if player is in leaderboard)
    bool hasPlayer(const std::string& playerName) const;

    // 获取玩家排名 (Get player rank)
    int getPlayerRank(const std::string& playerName) const;

private:
    // 排序记录（按分数降序）(Sort records by score descending)
    void sortRecords();

    // 限制记录数量 (Limit record count)
    void trimRecords();
};

#endif // LEADERBOARD_H
