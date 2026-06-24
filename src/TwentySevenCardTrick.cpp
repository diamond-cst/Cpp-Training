#include "TwentySevenCardTrick.h"

TwentySevenCardTrick::TwentySevenCardTrick()
    : ThreePileCardTrick(27, false, "TWENTY_SEVEN") {}

TwentySevenCardTrick::TwentySevenCardTrick(bool colors)
    : ThreePileCardTrick(27, colors, "TWENTY_SEVEN") {}

bool TwentySevenCardTrick::isDeckSizeAllowed(int cards) const {
    return cards == 27;
}

int TwentySevenCardTrick::practiceBaseScore() const {
    return 35;
}

int TwentySevenCardTrick::roundPauseMs() const {
    // 一千毫秒
    return 1000;
}
