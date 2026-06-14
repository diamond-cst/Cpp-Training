#ifndef TWENTYSEVENCARDTRICK_H
#define TWENTYSEVENCARDTRICK_H

#include "ThreePileCardTrick.h"

class TwentySevenCardTrick : public ThreePileCardTrick {
public:
    TwentySevenCardTrick();
    explicit TwentySevenCardTrick(bool colors);
    ~TwentySevenCardTrick() override = default;

protected:
    bool isDeckSizeAllowed(int cards) const override;
    int practiceBaseScore() const override;
    int roundPauseMs() const override;
};

#endif // TWENTYSEVENCARDTRICK_H
