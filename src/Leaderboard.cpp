#include "Leaderboard.h"
#include "Utils.h"
#include "Exceptions.h"
#include <fstream>
#include <iostream>
#include <iomanip>

// PlayerRecord构造函数
PlayerRecord::PlayerRecord()
    : name(""), score(0), gamesPlayed(0), correctGuesses(0), streak(0), timestamp("") {}

PlayerRecord::PlayerRecord(const std::string& n, int s, int gp, int cg, int st, const std::string& ts)
    : name(n), score(s), gamesPlayed(gp), correctGuesses(cg), streak(st), timestamp(ts) {}

// 比较运算符（按分数降序）
bool PlayerRecord::operator<(const PlayerRecord& other) const {
    if (score != other.score) {
        return score > other.score; // 分数高的排前面
    }
    return streak > other.streak; // 分数相同时，连胜多的排前面
}

bool PlayerRecord::operator>(const PlayerRecord& other) const {
    return other < *this;
}

// Leaderboard构造函数
Leaderboard::Leaderboard(const std::string& file) : filename(file) {
    load();
}

// 析构函数
Leaderboard::~Leaderboard() {
    save();
}

// 添加或更新玩家记录
void Leaderboard::addOrUpdateRecord(const PlayerRecord& record) {
    // 查找是否已存在该玩家
    auto it = std::find_if(records.begin(), records.end(),
                          [&record](const PlayerRecord& r) {
                              return r.name == record.name;
                          });

    if (it != records.end()) {
        // 更新现有记录
        it->score += record.score;
        it->gamesPlayed += record.gamesPlayed;
        it->correctGuesses += record.correctGuesses;
        it->streak = record.streak;
        it->timestamp = record.timestamp;
    } else {
        // 添加新记录
        records.push_back(record);
    }

    sortRecords();
    trimRecords();
}

// 获取排行榜，返回前count名玩家记录
std::vector<PlayerRecord> Leaderboard::getTopRecords(int count) const {
    // 获取实际排行榜数量
    int actualCount = std::min(count, static_cast<int>(records.size()));
    // 返回实际排行榜数量
    return std::vector<PlayerRecord>(records.begin(), records.begin() + actualCount);
}

// 获取玩家记录
PlayerRecord* Leaderboard::getPlayerRecord(const std::string& playerName) {
    // 查找玩家记录
    auto it = std::find_if(records.begin(), records.end(),
                          [&playerName](const PlayerRecord& r) {
                              return r.name == playerName;
                          });

    // 如果找到玩家记录，返回玩家记录指针
    if (it != records.end()) {
        return &(*it);
    }
    // 如果未找到玩家记录，返回空指针
    return nullptr;
}

// 显示排行榜
void Leaderboard::display() const {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                        排行榜前 " << std::setw(2) << MAX_RECORDS << " 名                       ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ 排名 │ 玩家名称        │ 分数  │ 游戏 │ 正确 │ 连胜 ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";

    if (records.empty()) {
        std::cout << "║                         暂无记录                              ║\n";
    } else {
        // 遍历排行榜前MAX_RECORDS名玩家记录
        for (size_t i = 0; i < records.size() && i < MAX_RECORDS; ++i) {
            // 获取当前玩家记录
            const auto& record = records[i];
            // 打印玩家排名、玩家名称、分数、游戏次数、正确次数、连胜次数
            std::cout << "║ " << std::setw(4) << (i + 1) << " │ "
                     << std::setw(15) << std::left << record.name.substr(0, 15) << std::right << " │ "
                     << std::setw(5) << record.score << " │ "
                     << std::setw(4) << record.gamesPlayed << " │ "
                     << std::setw(4) << record.correctGuesses << " │ "
                     << std::setw(4) << record.streak << " ║\n";
        }
    }

    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
}

// 显示带颜色的排行榜
void Leaderboard::displayColored() const {
    std::cout << "\n";
    Utils::printStyled("╔════════════════════════════════════════════════════════════════╗\n",
                      Utils::COLOR_CYAN, Utils::BOLD);
    Utils::printStyled("║                        排行榜前 10 名                         ║\n",
                      Utils::COLOR_CYAN, Utils::BOLD);
    Utils::printStyled("╠════════════════════════════════════════════════════════════════╣\n",
                      Utils::COLOR_CYAN, Utils::BOLD);
    std::cout << "║ 排名 │ 玩家名称        │ 分数  │ 游戏 │ 正确 │ 连胜 ║\n";
    Utils::printStyled("╠════════════════════════════════════════════════════════════════╣\n",
                      Utils::COLOR_CYAN, Utils::BOLD);

    if (records.empty()) {
        std::cout << "║                         暂无记录                              ║\n";
    } else {
        // 遍历排行榜前MAX_RECORDS名玩家记录
        for (size_t i = 0; i < records.size() && i < MAX_RECORDS; ++i) {
            const auto& record = records[i];

            // 根据排名选择颜色
            std::string color = Utils::COLOR_WHITE;
            if (i == 0) color = Utils::COLOR_YELLOW;      // 金色
            else if (i == 1) color = Utils::COLOR_CYAN;   // 银色
            else if (i == 2) color = Utils::COLOR_GREEN;  // 铜色

            // 打印玩家排名、玩家名称、分数、游戏次数、正确次数、连胜次数
            std::cout << "║ ";
            Utils::printColored(std::to_string(i + 1), color);
            std::cout << std::string(4 - std::to_string(i + 1).length(), ' ') << " │ "
                     << std::setw(15) << std::left << record.name.substr(0, 15) << std::right << " │ "
                     << std::setw(5) << record.score << " │ "
                     << std::setw(4) << record.gamesPlayed << " │ "
                     << std::setw(4) << record.correctGuesses << " │ "
                     << std::setw(4) << record.streak << " ║\n";
        }
    }

    Utils::printStyled("╚════════════════════════════════════════════════════════════════╝\n",
                      Utils::COLOR_CYAN, Utils::BOLD);
}

// 保存到文件
void Leaderboard::save() const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "写入排行榜");
    }

    // 保存记录数量 (Save record count)
    size_t count = records.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // 保存每条记录 (Save each record)
    for (const auto& record : records) {
        // 保存名称 (Save name)
        size_t nameLen = record.name.length();
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        file.write(record.name.c_str(), nameLen);

        // 保存数据 (Save data)
        file.write(reinterpret_cast<const char*>(&record.score), sizeof(record.score));
        file.write(reinterpret_cast<const char*>(&record.gamesPlayed), sizeof(record.gamesPlayed));
        file.write(reinterpret_cast<const char*>(&record.correctGuesses), sizeof(record.correctGuesses));
        file.write(reinterpret_cast<const char*>(&record.streak), sizeof(record.streak));

        // 保存时间戳 (Save timestamp)
        size_t tsLen = record.timestamp.length();
        file.write(reinterpret_cast<const char*>(&tsLen), sizeof(tsLen));
        file.write(record.timestamp.c_str(), tsLen);
    }

    file.close();
}

// 从文件加载 (Load from file)
void Leaderboard::load() {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // 文件不存在，创建空排行榜 (File doesn't exist, create empty leaderboard)
        records.clear();
        return;
    }

    records.clear();

    // 读取记录数量 (Read record count)
    size_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));

    // 读取每条记录 (Read each record)
    for (size_t i = 0; i < count; ++i) {
        PlayerRecord record;

        // 读取名称 (Read name)
        size_t nameLen;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        record.name.resize(nameLen);
        file.read(&record.name[0], nameLen);

        // 读取数据 (Read data)
        file.read(reinterpret_cast<char*>(&record.score), sizeof(record.score));
        file.read(reinterpret_cast<char*>(&record.gamesPlayed), sizeof(record.gamesPlayed));
        file.read(reinterpret_cast<char*>(&record.correctGuesses), sizeof(record.correctGuesses));
        file.read(reinterpret_cast<char*>(&record.streak), sizeof(record.streak));

        // 读取时间戳 (Read timestamp)
        size_t tsLen;
        file.read(reinterpret_cast<char*>(&tsLen), sizeof(tsLen));
        record.timestamp.resize(tsLen);
        file.read(&record.timestamp[0], tsLen);

        records.push_back(record);
    }

    file.close();
    sortRecords();
}

// 清空排行榜 (Clear leaderboard)
void Leaderboard::clear() {
    records.clear();
    save();
}

// 获取记录数量 (Get record count)
int Leaderboard::getRecordCount() const {
    return static_cast<int>(records.size());
}

// 检查玩家是否在排行榜中 (Check if player is in leaderboard)
bool Leaderboard::hasPlayer(const std::string& playerName) const {
    return std::find_if(records.begin(), records.end(),
                       [&playerName](const PlayerRecord& r) {
                           return r.name == playerName;
                       }) != records.end();
}

// 获取玩家排名 (Get player rank)
int Leaderboard::getPlayerRank(const std::string& playerName) const {
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].name == playerName) {
            return static_cast<int>(i + 1);
        }
    }
    return -1; // 未找到 (Not found)
}

// 排序记录（按分数降序）(Sort records by score descending)
void Leaderboard::sortRecords() {
    std::sort(records.begin(), records.end());
}

// 限制记录数量 (Limit record count)
void Leaderboard::trimRecords() {
    if (records.size() > MAX_RECORDS) {
        records.resize(MAX_RECORDS);
    }
}
