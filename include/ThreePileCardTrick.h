#ifndef THREEPILECARDTRICK_H
#define THREEPILECARDTRICK_H

#include "MagicTrick.h"
#include "PileChoice.h"
#include <string>
#include <vector>

// 三堆发牌类魔术的公共父类，封装 15/21/27 张牌魔术共用流程。
class ThreePileCardTrick : public MagicTrick {
protected:
    int deckSize;      // 当前玩法使用的总牌数
    int pileSize;      // 每一堆的牌数，始终等于 deckSize / 3
    int revealIndex;   // 三轮整理后默认揭晓的位置，下标从 0 开始

    Deck<Card> pile1;  // 第一堆，发牌时按顺序收集第 1、4、7... 张
    Deck<Card> pile2;  // 第二堆，发牌时按顺序收集第 2、5、8... 张
    Deck<Card> pile3;  // 第三堆，发牌时按顺序收集第 3、6、9... 张
    Card practiceTargetCard; // 练习模式中系统模拟观众记住的牌
    bool hasPracticeTarget;  // 是否已经生成有效的练习目标牌

    std::string saveType; // 存档中的玩法类型标识，用于防止读错存档
    bool configurableSave; // 是否在存档中额外保存牌数，供可配置玩法使用

public:
    // cards 决定总牌数，colors 决定显示颜色，type 用于存档类型校验。
    ThreePileCardTrick(int cards,
                       bool colors,
                       const std::string& type,
                       bool configurable = false);
    ~ThreePileCardTrick() override = default;

    void initialize() override;
    void performRound() override;
    bool isComplete() const override;
    void displayState() const override;
    std::string getName() const override;
    void reveal() override;
    void saveState(const std::string& filename) const override;
    void loadState(const std::string& filename) override;

    void setRememberedCard(const Card& card);
    Card getRememberedCard() const;
    int getDeckSize() const;

    void initializeForGui();
    void dealCurrentRoundForGui();
    std::vector<std::string> getWorkingDeckCardsForGui(bool revealFaces = false) const;
    std::vector<std::vector<std::string>> getCurrentPilesForGui(bool revealFaces = false) const;
    bool applyPileChoiceForGui(int chosenPile);
    bool finalizeRevealForGui(int selectedIndex, bool audienceConfirmedCorrect);
    int getRevealIndexForGui() const;
    int getPracticeTargetPileForGui() const;
    Card getCardAtForGui(int index) const;

protected:
    // 根据总牌数重新计算牌堆大小、揭晓位置，并重建工作牌组。
    void configureDeck(int cards);

    // 以下虚函数是给子类微调规则或提示文案的扩展点。
    virtual bool isDeckSizeAllowed(int cards) const;
    virtual int practiceBaseScore() const;
    virtual int roundPauseMs() const;
    virtual bool shouldShowPracticeHint() const;
    virtual std::string failureMessage() const;
    virtual PileChoice requestAudienceChoice(int roundNumber);
    virtual bool confirmAudienceReveal(const Card& revealedCard);

    // 初始化并洗牌，随后每轮把工作牌组分成三堆。
    void initializeCards();
    void dealIntoPiles();

    // 将观众选择的牌堆放回中间，这是三堆魔术能收敛到固定位置的核心。
    void reorganizePiles(int chosenPile);
    void displayPiles() const;

    // 两种输入流程：真实观众模式询问玩家，练习模式显示系统模拟的观众选择。
    PileChoice getUserChoice();
    PileChoice getPracticeChoice(int audiencePile);

    // 练习模式辅助函数，用于定位目标牌、验证揭晓位置和计算练习得分。
    int findPracticeTargetPile() const;
    bool isPracticeTargetAtRevealIndex() const;
    int getPracticeRevealIndex() const;
    void displayNumberedDeck() const;

    // 显示和回放辅助函数。
    void clearPiles();
    std::string pileToString(const Deck<Card>& pile) const;
    std::string displayCard(const Card& card) const;
    int calculatePracticeScore() const;

private:
    // 按当前顺序把某一堆追加回工作牌组。
    void appendPileToDeck(const Deck<Card>& pile);
};

#endif // THREEPILECARDTRICK_H
