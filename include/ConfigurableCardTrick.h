#ifndef CONFIGURABLECARDTRICK_H
#define CONFIGURABLECARDTRICK_H

#include "MagicTrick.h"
#include "Card.h"
#include "Deck.h"
#include <string>

class ReplayManager;

// 可配置牌数魔术类 (Configurable card trick class)
// 支持15/21/27张牌，均分为3堆，3轮后目标牌位于正中间。
class ConfigurableCardTrick : public MagicTrick {
private:
    int deckSize;                // 总牌数 (Total cards)
    int pileSize;                // 每堆牌数 (Cards per pile)
    int revealIndex;             // 揭示位置索引 (Reveal index)

    Deck<Card> pile1;
    Deck<Card> pile2;
    Deck<Card> pile3;

public:
    ConfigurableCardTrick();
    ConfigurableCardTrick(int cards, bool colors, bool animation, bool practiceMode);
    ~ConfigurableCardTrick() override = default;

    void initialize() override;
    void performRound() override;
    bool isComplete() const override;
    void displayState() const override;
    std::string getName() const override;
    void reveal() override;
    void saveState(const std::string& filename) const override;
    void loadState(const std::string& filename) override;

    int getDeckSize() const;

private:
    void configure(int cards);
    void initializeCards();
    void dealIntoPiles();
    void reorganizePiles(int chosenPile);
    void displayPiles() const;
    int getUserChoice();
    void clearPiles();
    std::string pileToString(const Deck<Card>& pile) const;
    std::string displayCard(const Card& card) const;
    int calculatePracticeScore() const;
};

#endif // CONFIGURABLECARDTRICK_H
