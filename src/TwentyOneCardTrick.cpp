#include "TwentyOneCardTrick.h"

// 构造函数，直接调用父类构造函数设置牌数为21，颜色选项和存档类型。
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
