#ifndef TWENTYONECARDTRICK_H
#define TWENTYONECARDTRICK_H

#include "MagicTrick.h"
#include "Card.h"
#include "Deck.h"
#include <string>
#include <vector>

class ReplayManager;

// 21张牌魔术类 (21 Card Trick class)
class TwentyOneCardTrick : public MagicTrick {
private:
    // 三个牌堆用于发牌 (Three piles for dealing)
    Deck<Card> pile1;
    Deck<Card> pile2;
    Deck<Card> pile3;

public:
    // 构造函数 (Constructor)
    TwentyOneCardTrick();
    explicit TwentyOneCardTrick(bool colors);

    // 析构函数 (Destructor)
    ~TwentyOneCardTrick() override = default;

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
    // 辅助函数：初始化21张牌 (Helper: initialize 21 cards)
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

#endif // TWENTYONECARDTRICK_H
