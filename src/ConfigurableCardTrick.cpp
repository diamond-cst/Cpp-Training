#include "ConfigurableCardTrick.h"

ConfigurableCardTrick::ConfigurableCardTrick()
    : ThreePileCardTrick(21, false, "CONFIGURABLE", true) {}

ConfigurableCardTrick::ConfigurableCardTrick(int cards, bool colors, bool animation, bool practiceMode)
    : ThreePileCardTrick(cards, colors, "CONFIGURABLE", true) {
    useAnimation = animation;
    magicianMode = practiceMode;
}

void ConfigurableCardTrick::configure(int cards) {
    // 检查 cards 是否合法
    // 更新 deckSize
    // 更新 pileSize
    // 更新 revealIndex
    // 重建 workingDeck
    // 重建 pile1、pile2、pile3
    configureDeck(cards);
}

bool ConfigurableCardTrick::isDeckSizeAllowed(int cards) const {
    return cards == 15 || cards == 21 || cards == 27;
}

bool ConfigurableCardTrick::shouldShowPracticeHint() const {
    return true;
}

std::string ConfigurableCardTrick::failureMessage() const {
    return "练习也会出错，下一次会更稳。";
}
