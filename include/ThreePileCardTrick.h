#ifndef THREEPILECARDTRICK_H
#define THREEPILECARDTRICK_H

#include "MagicTrick.h"
#include <string>

// 三堆发牌类魔术的公共父类
class ThreePileCardTrick : public MagicTrick {
protected:
    int deckSize;
    int pileSize;
    int revealIndex;

    Deck<Card> pile1;
    Deck<Card> pile2;
    Deck<Card> pile3;

    std::string saveType;
    bool configurableSave;

public:
    ThreePileCardTrick(int cards,
                       bool colors,
                       const std::string& type,
                       bool configurable = false);
    ~ThreePileCardTrick() override = default;

    void initialize() override;
    void performRound() override;
    bool isComplete() const override;
    void displayState() const override;
    std::string getName() const override;
    void reveal() override;
    void saveState(const std::string& filename) const override;
    void loadState(const std::string& filename) override;

    void setRememberedCard(const Card& card);
    Card getRememberedCard() const;
    int getDeckSize() const;

protected:
    void configureDeck(int cards);
    virtual bool isDeckSizeAllowed(int cards) const;
    virtual int practiceBaseScore() const;
    virtual int roundPauseMs() const;
    virtual bool shouldShowPracticeHint() const;
    virtual std::string failureMessage() const;

    void initializeCards();
    void dealIntoPiles();
    void reorganizePiles(int chosenPile);
    void displayPiles() const;
    int getUserChoice();
    void clearPiles();
    std::string pileToString(const Deck<Card>& pile) const;
    std::string displayCard(const Card& card) const;
    int calculatePracticeScore() const;

private:
    void appendPileToDeck(const Deck<Card>& pile);
};

#endif // THREEPILECARDTRICK_H
