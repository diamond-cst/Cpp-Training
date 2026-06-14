#ifndef TWENTYSEVENCARDTRICK_H
#define TWENTYSEVENCARDTRICK_H

#include "MagicTrick.h"
#include "Card.h"
#include "Deck.h"
#include <string>

class ReplayManager;

// 27张牌魔术类 (27 Card Trick class)
// 原理：27张牌分成3堆（每堆9张），进行3轮后，牌在第14张（索引13）
// Principle: 27 cards into 3 piles (9 cards each), after 3 rounds, card at position 14 (index 13)
class TwentySevenCardTrick : public MagicTrick {
private:
    // 三个牌堆用于发牌 (Three piles for dealing)
    Deck<Card> pile1;
    Deck<Card> pile2;
    Deck<Card> pile3;

public:
    // 构造函数 (Constructor)
    TwentySevenCardTrick();
    explicit TwentySevenCardTrick(bool colors);

    // 析构函数 (Destructor)
    ~TwentySevenCardTrick() override = default;

    // 实现基类的纯虚函数 (Implement base class pure virtual functions)
    void initialize() override;
    void performRound() override;
    bool isComplete() const override;
    void displayState() const override;
    std::string getName() const override;
    void reveal() override;
    void saveState(const std::string& filename) const override;
    void loadState(const std::string& filename) override;

    // 特定功能 (Specific functions)
    void setRememberedCard(const Card& card);
    Card getRememberedCard() const;

private:
    // 辅助函数：初始化27张牌 (Helper: initialize 27 cards)
    void initializeCards();

    // 辅助函数：发牌到三堆 (Helper: deal cards into three piles)
    void dealIntoPiles();

    // 辅助函数：重新整理牌堆 (Helper: reorganize piles)
    void reorganizePiles(int chosenPile);

    // 辅助函数：显示三个牌堆 (Helper: display three piles)
    void displayPiles() const;

    // 辅助函数：获取用户选择的牌堆 (Helper: get user's chosen pile)
    int getUserChoice();

    // 辅助函数：清空牌堆 (Helper: clear piles)
    void clearPiles();

    // 辅助函数：牌堆转字符串 (Helper: pile to string)
    std::string pileToString(const Deck<Card>& pile) const;

    std::string displayCard(const Card& card) const;
    int calculatePracticeScore() const;
};

#endif // TWENTYSEVENCARDTRICK_H
