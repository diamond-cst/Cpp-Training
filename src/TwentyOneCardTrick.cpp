#include "TwentyOneCardTrick.h"

TwentyOneCardTrick::TwentyOneCardTrick()
    : ThreePileCardTrick(21, false, "TWENTY_ONE") {}

TwentyOneCardTrick::TwentyOneCardTrick(bool colors)
    : ThreePileCardTrick(21, colors, "TWENTY_ONE") {}

bool TwentyOneCardTrick::isDeckSizeAllowed(int cards) const {
    return cards == 21;
}

int TwentyOneCardTrick::practiceBaseScore() const {
    return 30;
}
