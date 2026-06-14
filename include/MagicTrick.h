#ifndef MAGICTRICK_H
#define MAGICTRICK_H

#include "Card.h"
#include "Deck.h"
#include <chrono>
#include <string>

class ReplayManager;

// 魔术抽象基类 (Abstract base class for magic tricks)
class MagicTrick {
protected:
    Deck<Card> workingDeck;      // 工作牌堆 (Working deck)
    int currentRound;            // 当前轮数 (Current round)
    Card rememberedCard;         // 观众记住的牌 (Remembered card)
    std::string playerName;      // 玩家名称 (Player name)
    int score;                   // 分数 (Score)
    bool useColors;              // 是否使用颜色 (Use colors)
    bool useAnimation;           // 是否使用动画 (Use animation)
    bool magicianMode;           // 魔术师练习模式 (Magician practice mode)
    bool soundEnabled;           // 是否启用提示音 (Sound enabled)
    bool hideFaces;              // 是否隐藏牌面 (Hide card faces)
    bool numericCards;           // 是否使用数字牌 (Use numeric cards)
    bool lastGuessCorrect;       // 最近一次揭示是否正确 (Last reveal correct)
    bool hasRevealResult;        // 是否已有揭示结果 (Has reveal result)
    int elapsedSeconds;          // 本局用时 (Elapsed seconds)
    std::chrono::steady_clock::time_point startTime;
    ReplayManager* replayManager;// 回放管理器 (Replay manager)

    explicit MagicTrick(int initialDeckCapacity = 52, bool colors = false)
        : workingDeck(initialDeckCapacity), currentRound(0), rememberedCard(),
          playerName("Player"), score(0), useColors(colors), useAnimation(false),
          magicianMode(false), soundEnabled(false), hideFaces(false), numericCards(false),
          lastGuessCorrect(false), hasRevealResult(false), elapsedSeconds(0),
          startTime(std::chrono::steady_clock::now()), replayManager(nullptr) {}

public:
    // 虚析构函数 (Virtual destructor)
    virtual ~MagicTrick() = default;

    // 纯虚函数：初始化魔术 (Pure virtual: initialize trick)
    virtual void initialize() = 0;

    // 纯虚函数：执行一轮 (Pure virtual: perform one round)
    virtual void performRound() = 0;

    // 纯虚函数：检查是否完成 (Pure virtual: check if complete)
    virtual bool isComplete() const = 0;

    // 纯虚函数：显示当前状态 (Pure virtual: display current state)
    virtual void displayState() const = 0;

    // 纯虚函数：获取魔术名称 (Pure virtual: get trick name)
    virtual std::string getName() const = 0;

    // 纯虚函数：揭示结果 (Pure virtual: reveal result)
    virtual void reveal() = 0;

    // 纯虚函数：保存状态 (Pure virtual: save state)
    virtual void saveState(const std::string& filename) const = 0;

    // 纯虚函数：加载状态 (Pure virtual: load state)
    virtual void loadState(const std::string& filename) = 0;

    // 通用状态访问 (Common state access)
    virtual int getCurrentRound() const { return currentRound; }

    virtual void setPlayerName(const std::string& name) { playerName = name; }

    virtual std::string getPlayerName() const { return playerName; }

    virtual int getScore() const { return score; }
    virtual void addScore(int points) { score += points; }
    virtual void setUseAnimation(bool enabled) { useAnimation = enabled; }
    virtual void setMagicianMode(bool enabled) { magicianMode = enabled; }
    virtual void setSoundEnabled(bool enabled) { soundEnabled = enabled; }
    virtual void setHideFaces(bool enabled) { hideFaces = enabled; }
    virtual void setNumericCards(bool enabled) { numericCards = enabled; }
    virtual void setReplayManager(ReplayManager* manager) { replayManager = manager; }
    virtual bool wasLastGuessCorrect() const { return lastGuessCorrect; }
    virtual bool hasResult() const { return hasRevealResult; }
    virtual int getElapsedSeconds() const { return elapsedSeconds; }
};

#endif // MAGICTRICK_H
