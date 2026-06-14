#ifndef CONFIGURABLECARDTRICK_H
#define CONFIGURABLECARDTRICK_H

#include "ThreePileCardTrick.h"

class ConfigurableCardTrick : public ThreePileCardTrick {
public:
    ConfigurableCardTrick();
    ConfigurableCardTrick(int cards, bool colors, bool animation, bool practiceMode);
    ~ConfigurableCardTrick() override = default;

    void configure(int cards);

protected:
    bool isDeckSizeAllowed(int cards) const override;
    bool shouldShowPracticeHint() const override;
    std::string failureMessage() const override;
};

#endif // CONFIGURABLECARDTRICK_H
