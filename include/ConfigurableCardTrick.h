#ifndef CONFIGURABLECARDTRICK_H
#define CONFIGURABLECARDTRICK_H

#include "ThreePileCardTrick.h"

class ConfigurableCardTrick : public ThreePileCardTrick {
public:
    ConfigurableCardTrick();
    ConfigurableCardTrick(int cards, bool colors, bool animation, bool practiceMode);
    ~ConfigurableCardTrick() override = default;

    // 允许在游戏过程中重新配置牌数
    void configure(int cards);

protected:
    // 检查牌数是否为 15、21 或 27
    bool isDeckSizeAllowed(int cards) const override;
    // 显示提示
    bool shouldShowPracticeHint() const override;
    // 揭示失败时的提示文案
    std::string failureMessage() const override;
};

#endif // CONFIGURABLECARDTRICK_H
