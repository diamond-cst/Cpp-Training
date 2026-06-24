#ifndef TWENTYONECARDTRICK_H
#define TWENTYONECARDTRICK_H

#include "ThreePileCardTrick.h"

class TwentyOneCardTrick : public ThreePileCardTrick {
public:
    TwentyOneCardTrick();
    explicit TwentyOneCardTrick(bool colors);
    ~TwentyOneCardTrick() override = default;

protected:
    // 检查牌数是否为 21
    bool isDeckSizeAllowed(int cards) const override;
    // 练习模式的基础分数
    int practiceBaseScore() const override;
};

#endif // TWENTYONECARDTRICK_H
