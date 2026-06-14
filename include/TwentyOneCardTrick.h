#ifndef TWENTYONECARDTRICK_H
#define TWENTYONECARDTRICK_H

#include "ThreePileCardTrick.h"

class TwentyOneCardTrick : public ThreePileCardTrick {
public:
    TwentyOneCardTrick();
    explicit TwentyOneCardTrick(bool colors);
    ~TwentyOneCardTrick() override = default;

protected:
    bool isDeckSizeAllowed(int cards) const override;
    int practiceBaseScore() const override;
};

#endif // TWENTYONECARDTRICK_H
