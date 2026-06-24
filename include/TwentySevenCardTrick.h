#ifndef TWENTYSEVENCARDTRICK_H
#define TWENTYSEVENCARDTRICK_H

#include "ThreePileCardTrick.h"

class TwentySevenCardTrick : public ThreePileCardTrick {
public:
    TwentySevenCardTrick();
    explicit TwentySevenCardTrick(bool colors);
    ~TwentySevenCardTrick() override = default;

protected:
    // 检查牌数是否为 27
    bool isDeckSizeAllowed(int cards) const override;
    // 练习模式的基础分数
    int practiceBaseScore() const override;
    // 控制 27 张牌魔术每轮动画结束后的等待时间
    int roundPauseMs() const override;
};

#endif // TWENTYSEVENCARDTRICK_H
