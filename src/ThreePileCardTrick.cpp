#include "ThreePileCardTrick.h"
#include "Exceptions.h"
#include "PileChoice.h"
#include "ReplayManager.h"
#include "Utils.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>

namespace {
const int SAVE_MAGIC = 0x4D545243; // MTTC

// 二进制存档中字符串按“长度 + 内容”写入，读取时才能准确知道要读多少字节。
void writeString(std::ofstream& file, const std::string& value) {
    size_t length = value.length();
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.c_str(), length);
}

// 读取存档字符串时限制最大长度，避免损坏存档导致申请过大的内存。
std::string readString(std::ifstream& file) {
    size_t length = 0;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (!file || length > 1024) {
        throw InvalidGameStateException("存档中的字符串长度无效");
    }

    std::string value(length, '\0'); // 创建一个长度为 length 的字符串，并用 null 字符初始化
    if (length > 0) {
        file.read(&value[0], length);
    }
    if (!file) {
        throw InvalidGameStateException("读取存档字符串失败");
    }
    return value;
}
}

ThreePileCardTrick::ThreePileCardTrick(int cards,
                                       bool colors,
                                       const std::string& type,
                                       bool configurable)
    : MagicTrick(cards, colors),
      deckSize(cards),
      pileSize(cards / 3),
      revealIndex(cards / 2),
      pile1(cards / 3),
      pile2(cards / 3),
      pile3(cards / 3),
      practiceTargetCard(),
      hasPracticeTarget(false),
      saveType(type),
      configurableSave(configurable) {
    // 构造时先根据牌数统一设置牌堆容量和最终揭晓位置。
    configureDeck(cards);
}

void ThreePileCardTrick::configureDeck(int cards) {
    if (!isDeckSizeAllowed(cards)) {
        throw InvalidInputException("可配置魔术只支持15、21或27张牌");
    }

    deckSize = cards;
    pileSize = cards / 3;
    revealIndex = cards / 2;
    // 重新创建工作牌组和三个牌堆，保证切换牌数后容量也同步更新。
    workingDeck = Deck<Card>(deckSize);
    pile1 = Deck<Card>(pileSize);
    pile2 = Deck<Card>(pileSize);
    pile3 = Deck<Card>(pileSize);
}

bool ThreePileCardTrick::isDeckSizeAllowed(int cards) const {
    return cards == 15 || cards == 21 || cards == 27;
}

// 练习模式的基础分数根据牌数调整，牌越多分数越高，增加挑战性。
int ThreePileCardTrick::practiceBaseScore() const {
    return 20 + deckSize / 2;
}

// 每轮结束后暂停的时间也根据牌数调整，牌越多暂停越久，给玩家更多时间观察牌堆。
int ThreePileCardTrick::roundPauseMs() const {
    return 800;
}

// 练习模式是否显示提示文案，三堆魔术的练习难度适中，可以选择不显示额外提示。
bool ThreePileCardTrick::shouldShowPracticeHint() const {
    return false;
}

// 揭示失败时的提示文案，三堆魔术的失败提示可以相对简单直接。
std::string ThreePileCardTrick::failureMessage() const {
    return "哦不！出错了...";
}

// 询问玩家观众选择了哪个牌堆，三堆魔术的交互相对简单，直接让玩家输入1/2/3即可。
PileChoice ThreePileCardTrick::requestAudienceChoice(int) {
    return getUserChoice();
}

// 揭示后询问玩家是否猜对了，三堆魔术的揭示验证可以直接让玩家确认。
bool ThreePileCardTrick::confirmAudienceReveal(const Card&) {
    return Utils::confirm("我猜对了吗？");
}

// 初始化时根据当前牌数生成对应的牌组，标准牌模式用三种花色的牌，数字牌模式用1到deckSize的数字。
void ThreePileCardTrick::initialize() {
    currentRound = 0;
    score = 0;
    hasRevealResult = false;
    lastGuessCorrect = false;
    saveAndExitRequested = false;
    practiceMistakes = 0;
    hasPracticeTarget = false;
    elapsedSeconds = 0;
    startTime = std::chrono::steady_clock::now();
    workingDeck.clear();
    clearPiles();
    initializeCards();
    if (magicianMode && workingDeck.getSize() > 0) {
        // 练习模式由系统随机选一张牌作为“观众记住的牌”，玩家需要按流程找回它。
        static std::mt19937 rng(std::random_device{}());
        // 创建一个整数随机分布
        std::uniform_int_distribution<int> dist(0, workingDeck.getSize() - 1);
        // 这里先生成一个随机下标，再从工作牌组中取出对应的牌作为练习目标牌
        practiceTargetCard = workingDeck.getCard(dist(rng));
        hasPracticeTarget = true;
    }

    Utils::clearScreen();
    Utils::printTitle(getName());
    Utils::printColored("玩家: ", Utils::COLOR_CYAN);
    std::cout << playerName << "\n";
    Utils::printColored("模式: ", Utils::COLOR_CYAN);
    std::cout << (magicianMode ? "魔术师练习" : "观众互动") << "\n\n";

    Utils::printColored("这是" + std::to_string(deckSize) + "张牌:\n", Utils::COLOR_GREEN);
    displayState();

    if (magicianMode && hasPracticeTarget) {
        Utils::printColored("\n系统已模拟一位观众记住了一张牌。\n",
                            Utils::COLOR_YELLOW);
        Utils::printColored("每轮系统会告诉你观众选择的牌堆，请快速输入并完成整理。\n",
                            Utils::COLOR_YELLOW);
    } else {
        Utils::printColored("\n请在心中记住其中一张牌，但不要告诉我！\n", Utils::COLOR_YELLOW);
    }
    Utils::pressAnyKey();
}

void ThreePileCardTrick::initializeCards() {
    if (numericCards) {
        // 数字牌模式直接生成 1 到 deckSize，适合 15/21/27 张可配置魔术。
        for (int value = 1; value <= deckSize; ++value) {
            workingDeck.addCard(Card(value));
        }
        workingDeck.shuffle();
        return;
    }

    // 标准牌模式用三种花色的牌，花色顺序是红桃、方块、梅花，每种花色有 pileSize 张牌，点数从 A 到对应的 pileSize。
    Card::Suit suits[] = {Card::Suit::HEARTS, Card::Suit::DIAMONDS, Card::Suit::CLUBS};
    // 标准牌模式只需要 deckSize 张牌，因此用三种花色各生成 pileSize 张。
    for (int s = 0; s < 3; ++s) {
        for (int r = 1; r <= pileSize; ++r) {
            workingDeck.addCard(Card(suits[s], static_cast<Card::Rank>(r)));
        }
    }

    workingDeck.shuffle();
}

void ThreePileCardTrick::initializeForGui() {
    currentRound = 0;
    score = 0;
    hasRevealResult = false;
    lastGuessCorrect = false;
    saveAndExitRequested = false;
    practiceMistakes = 0;
    hasPracticeTarget = false;
    elapsedSeconds = 0;
    startTime = std::chrono::steady_clock::now();
    workingDeck.clear();
    clearPiles();
    initializeCards();

    if (magicianMode && workingDeck.getSize() > 0) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, workingDeck.getSize() - 1);
        practiceTargetCard = workingDeck.getCard(dist(rng));
        hasPracticeTarget = true;
    }
}

void ThreePileCardTrick::dealCurrentRoundForGui() {
    if (isComplete()) {
        throw InvalidGameStateException("魔术已经完成");
    }
    dealIntoPiles();
}

std::vector<std::string> ThreePileCardTrick::getWorkingDeckCardsForGui(bool revealFaces) const {
    std::vector<std::string> cards;
    cards.reserve(workingDeck.getSize());
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        cards.push_back((hideFaces && !revealFaces)
            ? Utils::hiddenCardFace()
            : workingDeck.getCard(i).toString(false));
    }
    return cards;
}

std::vector<std::vector<std::string>> ThreePileCardTrick::getCurrentPilesForGui(bool revealFaces) const {
    std::vector<std::vector<std::string>> result(3);
    const Deck<Card>* sourcePiles[] = {&pile1, &pile2, &pile3};
    for (int pileIndex = 0; pileIndex < 3; ++pileIndex) {
        result[pileIndex].reserve(sourcePiles[pileIndex]->getSize());
        for (int i = 0; i < sourcePiles[pileIndex]->getSize(); ++i) {
            result[pileIndex].push_back((hideFaces && !revealFaces)
                ? Utils::hiddenCardFace()
                : sourcePiles[pileIndex]->getCard(i).toString(false));
        }
    }
    return result;
}

bool ThreePileCardTrick::applyPileChoiceForGui(int chosenPile) {
    if (chosenPile < 1 || chosenPile > 3) {
        throw InvalidInputException("牌堆编号必须是1到3");
    }
    if (isComplete()) {
        throw InvalidGameStateException("魔术已经完成");
    }

    const int roundNumber = currentRound + 1;
    bool practiceChoiceCorrect = true;
    if (magicianMode) {
        int audiencePile = findPracticeTargetPile();
        if (audiencePile < 1 || audiencePile > 3) {
            throw InvalidGameStateException("练习目标牌不在任何牌堆中");
        }
        practiceChoiceCorrect = chosenPile == audiencePile;
        if (!practiceChoiceCorrect) {
            ++practiceMistakes;
        }
    }

    if (replayManager) {
        replayManager->recordRound(roundNumber,
                                   pileToString(pile1),
                                   pileToString(pile2),
                                   pileToString(pile3),
                                   chosenPile);
    }

    reorganizePiles(chosenPile);
    currentRound = roundNumber;
    clearPiles();
    return practiceChoiceCorrect;
}

bool ThreePileCardTrick::finalizeRevealForGui(int selectedIndex, bool audienceConfirmedCorrect) {
    if (!isComplete()) {
        throw InvalidGameStateException("三轮完成前不能揭晓");
    }
    if (selectedIndex < 0 || selectedIndex >= workingDeck.getSize()) {
        throw OutOfBoundsException(selectedIndex, workingDeck.getSize());
    }

    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    bool correct = magicianMode
        ? (hasPracticeTarget && workingDeck[selectedIndex] == practiceTargetCard)
        : audienceConfirmedCorrect;

    lastGuessCorrect = correct;
    hasRevealResult = true;
    addScore(correct ? (magicianMode ? calculatePracticeScore() : 10) : -5);

    if (replayManager) {
        replayManager->recordReveal(workingDeck[selectedIndex].toString(false), correct, score);
    }

    return correct;
}

int ThreePileCardTrick::getRevealIndexForGui() const {
    return revealIndex;
}

int ThreePileCardTrick::getPracticeTargetPileForGui() const {
    return findPracticeTargetPile();
}

Card ThreePileCardTrick::getCardAtForGui(int index) const {
    return workingDeck.getCard(index);
}

void ThreePileCardTrick::dealIntoPiles() {
    clearPiles();

    // 按 1、2、3、1、2、3 的顺序发牌，模拟魔术表演中的三列发牌。
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        if (i % 3 == 0) {
            pile1.addCard(workingDeck[i]);
        } else if (i % 3 == 1) {
            pile2.addCard(workingDeck[i]);
        } else {
            pile3.addCard(workingDeck[i]);
        }
    }
}

void ThreePileCardTrick::displayPiles() const {
    // 延迟35毫秒
    const int dealDelayMs = 35;

    std::cout << "\n";
    if (useAnimation) {
        Utils::printColored("正在发牌，请观察每张牌的位置...\n", Utils::COLOR_YELLOW);
        std::cout << "\n";
    }

    // 创建了一个数组，里面放的是三个牌堆的地址，这样后续遍历输出时就可以通过索引访问对应的牌堆
    const Deck<Card>* piles[] = {&pile1, &pile2, &pile3};
    for (int pileIndex = 0; pileIndex < 3; ++pileIndex) {
        Utils::printColored("牌堆 " + std::to_string(pileIndex + 1) + ":  ", Utils::COLOR_CYAN);
        for (int i = 0; i < piles[pileIndex]->getSize(); ++i) {
            std::cout << displayCard(piles[pileIndex]->getCard(i)) << " " << std::flush;
            if (soundEnabled && useAnimation) {
                Utils::playDealSound();
            }
            if (useAnimation) {
                Utils::sleep(dealDelayMs);
            }
        }
        std::cout << "\n";
    }
}

PileChoice ThreePileCardTrick::getUserChoice() {
    while (true) {
        std::cout << "\n你记住的牌在哪一堆？\n";
        std::cout << "输入 1/2/3 选择牌堆，P 暂停，S 保存并返回主菜单。\n";
        std::cout << "选择: ";

        try {
            PileChoice choice;
            std::cin >> choice;
            if (choice.isPauseRequested()) {
                Utils::waitForResume(); // 暂停后继续循环询问输入
                continue;
            }
            return choice; // 只有当不是暂停请求时才返回选择结果
        } catch (const InvalidInputException& e) {
            if (std::cin.eof()) {
                throw;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Utils::printColored(std::string("无效输入: ") + e.what() + "\n", Utils::COLOR_RED);
        }
    }
}

PileChoice ThreePileCardTrick::getPracticeChoice(int audiencePile) {
    while (true) {
        std::cout << "\n模拟观众: 我的牌在第 " << audiencePile << " 堆。\n";
        std::cout << "输入 1/2/3 选择牌堆，P 暂停，S 保存并返回主菜单。\n";
        std::cout << "选择: ";

        try {
            PileChoice choice;
            std::cin >> choice;
            if (choice.isPauseRequested()) {
                Utils::waitForResume();
                continue;
            }
            return choice;
        } catch (const InvalidInputException& e) {
            if (std::cin.eof()) {
                throw;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Utils::printColored(std::string("无效输入: ") + e.what() + "\n", Utils::COLOR_RED);
        }
    }
}

int ThreePileCardTrick::findPracticeTargetPile() const {
    if (!hasPracticeTarget) {
        return 0;
    }

    // 练习模式下系统知道目标牌，因此可以自动告诉玩家“观众选择了哪一堆”。
    const Deck<Card>* piles[] = {&pile1, &pile2, &pile3};
    for (int pileIndex = 0; pileIndex < 3; ++pileIndex) {
        for (int i = 0; i < piles[pileIndex]->getSize(); ++i) {
            if (piles[pileIndex]->getCard(i) == practiceTargetCard) {
                return pileIndex + 1;
            }
        }
    }
    return 0;
}

bool ThreePileCardTrick::isPracticeTargetAtRevealIndex() const {
    return hasPracticeTarget
        && revealIndex >= 0
        && revealIndex < workingDeck.getSize()
        && workingDeck.getCard(revealIndex) == practiceTargetCard;
}

int ThreePileCardTrick::getPracticeRevealIndex() const {
    while (true) {
        std::cout << "\n请选择你要揭晓的牌序号（1-" << workingDeck.getSize()
                  << "），输入 P 可暂停。\n";
        std::cout << "揭晓: ";

        std::string token;
        std::cin >> token;
        if (std::cin.fail()) {
            if (std::cin.eof()) {
                throw InvalidInputException("读取揭晓牌序号时输入已结束");
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Utils::printColored("无效输入！请输入数字。\n", Utils::COLOR_RED);
            continue;
        }

        if (token == "P" || token == "p") {
            Utils::waitForResume();
            continue;
        }

        std::istringstream iss(token);
        int value = 0;
        char extra = '\0';
        // 使用字符串流校验完整输入，避免 "3abc" 这类内容被当成有效数字。
        if ((iss >> value) && !(iss >> extra) && value >= 1 && value <= workingDeck.getSize()) {
            return value - 1;
        }

        Utils::printColored("输入超出范围！\n", Utils::COLOR_RED);
        std::cout << "请输入 1 到 " << workingDeck.getSize() << " 之间的数字。\n";
    }
}

// 把最终牌组按序号显示出来，方便练习模式下让玩家选择要揭晓第几张牌
void ThreePileCardTrick::displayNumberedDeck() const {
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        std::cout << (i + 1) << ":"
                  << (hideFaces ? Utils::hiddenCardFace() : workingDeck.getCard(i).toString(useColors))
                  << "  ";
        if ((i + 1) % pileSize == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

void ThreePileCardTrick::appendPileToDeck(const Deck<Card>& pile) {
    for (int i = 0; i < pile.getSize(); ++i) {
        workingDeck.addCard(pile[i]);
    }
}

void ThreePileCardTrick::reorganizePiles(int chosenPile) {
    workingDeck.clear();

    // 三堆魔术的关键步骤：把观众选中的牌堆放到中间，重复三轮后目标牌会收敛到固定位置。
    if (chosenPile == 1) {
        appendPileToDeck(pile2);
        appendPileToDeck(pile1);
        appendPileToDeck(pile3);
    } else if (chosenPile == 2) {
        appendPileToDeck(pile1);
        appendPileToDeck(pile2);
        appendPileToDeck(pile3);
    } else {
        appendPileToDeck(pile1);
        appendPileToDeck(pile3);
        appendPileToDeck(pile2);
    }
}

void ThreePileCardTrick::clearPiles() {
    pile1.clear();
    pile2.clear();
    pile3.clear();
}

// 辅助函数：把牌堆转换成字符串用于显示和回放记录。
std::string ThreePileCardTrick::pileToString(const Deck<Card>& pile) const {
    std::ostringstream oss;
    for (int i = 0; i < pile.getSize(); ++i) {
        if (i > 0) {
            oss << " ";
        }
        oss << pile.getCard(i).toString(false);
    }
    return oss.str();
}

std::string ThreePileCardTrick::displayCard(const Card& card) const {
    return hideFaces ? Utils::hiddenCardFace() : card.toString(useColors);
}

// 最终分数 = 基础分 - 时间扣分 - 错误扣分
int ThreePileCardTrick::calculatePracticeScore() const {
    int bonus = practiceBaseScore() - elapsedSeconds / 2 - practiceMistakes * 5;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

// 进行一轮表演，包含发牌、询问选择、整理牌堆等流程。练习模式下系统会模拟观众选择，并根据玩家输入判断是否正确。
void ThreePileCardTrick::performRound() {
    if (isComplete()) {
        throw InvalidGameStateException("魔术已经完成");
    }

    int roundNumber = currentRound + 1;

    Utils::clearScreen();
    Utils::printSeparator('=', 60);
    Utils::printStyled("  第 " + std::to_string(roundNumber) + " 轮\n",
                       Utils::COLOR_YELLOW, Utils::BOLD);
    Utils::printSeparator('=', 60);

    dealIntoPiles(); // 发牌到三个牌堆
    displayPiles(); // 显示三个牌堆，练习模式下系统会提示观众选择了哪一堆

    PileChoice choice;
    int chosenPile = 0;
    if (magicianMode) {
        // 练习模式不询问真实观众，而是根据系统目标牌所在牌堆模拟观众回答。
        chosenPile = findPracticeTargetPile();
        if (chosenPile < 1 || chosenPile > 3) {
            throw InvalidGameStateException("练习目标牌不在任何牌堆中");
        }
        Utils::printColored("\n系统模拟观众已选择第 " + std::to_string(chosenPile) + " 堆。\n",
                            Utils::COLOR_YELLOW);
        if (shouldShowPracticeHint()) {
            Utils::printColored("练习提示: 选中的牌堆会被放到中间。\n", Utils::COLOR_MAGENTA);
        }
        choice = getPracticeChoice(chosenPile);
    } else {
        choice = requestAudienceChoice(roundNumber);
        chosenPile = choice.getValue();
    }

    if (choice.isSaveRequested()) {
        // 保存请求只标记状态并退出本轮，真正保存由外层游戏流程处理。
        saveAndExitRequested = true;
        clearPiles();
        Utils::printColored("\n已准备保存当前进度。\n", Utils::COLOR_GREEN);
        return;
    }

    if (magicianMode) {
        // 练习时如果玩家选错牌堆，仍按玩家选择继续流程，但记录失误用于最终扣分。
        if (choice.getValue() == chosenPile) {
            Utils::printColored("\n输入正确，已按观众选择整理牌堆。\n",
                                Utils::COLOR_GREEN);
        } else {
            ++practiceMistakes;
            Utils::printColored("\n输入错误。观众选择的是第 " + std::to_string(chosenPile) +
                                " 堆，你选择了第 " + std::to_string(choice.getValue()) + " 堆。\n",
                                Utils::COLOR_RED);
            chosenPile = choice.getValue();
        }
    }

    if (replayManager) {
        // 回放记录的是每轮展示给玩家的三堆内容和最终选择，便于之后复盘流程。
        replayManager->recordRound(roundNumber,
                                   pileToString(pile1),
                                   pileToString(pile2),
                                   pileToString(pile3),
                                   chosenPile);
    }

    reorganizePiles(chosenPile);
    currentRound = roundNumber;

    Utils::printColored("\n牌已重新整理！\n", Utils::COLOR_GREEN);
    if (useAnimation) {
        Utils::sleep(roundPauseMs());
    }
}

bool ThreePileCardTrick::isComplete() const {
    return currentRound >= 3;
}

// 显示当前牌组状态，主要用于初始化时让玩家记牌。三堆魔术的牌组状态直接按顺序显示即可，不需要区分牌堆。
void ThreePileCardTrick::displayState() const {
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        std::cout << displayCard(workingDeck.getCard(i)) << " ";
        if ((i + 1) % pileSize == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

std::string ThreePileCardTrick::getName() const {
    return std::to_string(deckSize) + "张牌魔术";
}

// 揭晓时刻，展示玩家记住的牌
void ThreePileCardTrick::reveal() {
    if (!isComplete()) {
        throw InvalidGameStateException("三轮完成前不能揭晓");
    }

    Utils::clearScreen();
    Utils::printTitle("揭晓时刻！");
    int selectedIndex = revealIndex;
    if (magicianMode && hasPracticeTarget) {
        // 练习模式要求玩家自己选择揭晓位置，用来检验是否真正掌握三堆魔术流程。
        Utils::printColored("三轮整理完成，请从最终牌列中选择你要揭晓的牌。\n",
                            Utils::COLOR_YELLOW);
        std::cout << "\n";
        displayNumberedDeck();
        selectedIndex = getPracticeRevealIndex();
        Utils::printColored("\n你选择揭晓第" + std::to_string(selectedIndex + 1) + "张牌...\n",
                            Utils::COLOR_YELLOW);
    } else {
        Utils::printColored("你记住的牌是第" + std::to_string(revealIndex + 1) + "张牌...\n",
                            Utils::COLOR_YELLOW);
    }

    if (soundEnabled) {
        Utils::playRevealSound();
    }
    std::cout << "\n";

    Utils::printStyled("它是: " + workingDeck[selectedIndex].toString(useColors) + "\n",
                       Utils::COLOR_GREEN, Utils::BOLD);
    std::cout << "\n";

    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    // 观众模式由观众确认结果；练习模式直接比较系统目标牌，避免人为确认带来的误差。
    bool correct = magicianMode
        ? (hasPracticeTarget && workingDeck[selectedIndex] == practiceTargetCard)
        : confirmAudienceReveal(workingDeck[selectedIndex]);
    lastGuessCorrect = correct;
    hasRevealResult = true;

    if (correct) {
        int points = magicianMode ? calculatePracticeScore() : 10;
        addScore(points);
        Utils::printColored(magicianMode ? "\n练习完成！流程正确。\n" : "\n太好了！魔术成功！\n",
                            Utils::COLOR_GREEN);
        Utils::printColored("得分 +" + std::to_string(points) + "! 当前分数: " +
                            std::to_string(score) + "，用时: " +
                            std::to_string(elapsedSeconds) + " 秒\n", Utils::COLOR_YELLOW);
        std::cout << std::flush;
        if (soundEnabled) {
            Utils::playResultSound(true);
        }
    } else {
        addScore(-5);
        Utils::printColored("\n" + failureMessage() + "\n", Utils::COLOR_RED);
        Utils::printColored("得分 -5. 当前分数: " + std::to_string(score) + "\n",
                            Utils::COLOR_YELLOW);
        std::cout << std::flush;
        if (soundEnabled) {
            Utils::playResultSound(false);
        }
    }

    if (replayManager) {
        replayManager->recordReveal(workingDeck[selectedIndex].toString(false), correct, score);
    }
}

// 把当前三堆牌魔术的游戏进度保存到文件里
void ThreePileCardTrick::saveState(const std::string& filename) const {
    // 用二进制方式打开输出文件
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "写入存档");
    }

    // 新版存档先写入魔数和魔术类型，用来识别文件格式并防止加载到错误玩法。
    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC), sizeof(SAVE_MAGIC));
    writeString(file, saveType);
    if (configurableSave) {
        file.write(reinterpret_cast<const char*>(&deckSize), sizeof(deckSize));
    }
    writeString(file, playerName);
    file.write(reinterpret_cast<const char*>(&currentRound), sizeof(currentRound));
    file.write(reinterpret_cast<const char*>(&score), sizeof(score));
    file.write(reinterpret_cast<const char*>(&useColors), sizeof(useColors));
    file.write(reinterpret_cast<const char*>(&useAnimation), sizeof(useAnimation));
    file.write(reinterpret_cast<const char*>(&magicianMode), sizeof(magicianMode));

    int savedDeckSize = workingDeck.getSize();
    file.write(reinterpret_cast<const char*>(&savedDeckSize), sizeof(savedDeckSize));
    // 每张牌同时保存数值、花色、点数，数字牌和标准牌都能恢复。
    for (int i = 0; i < savedDeckSize; ++i) {
        Card card = workingDeck.getCard(i);
        int value = card.getValue();
        Card::Suit suit = card.getSuit();
        Card::Rank rank = card.getRank();
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
        file.write(reinterpret_cast<const char*>(&suit), sizeof(suit));
        file.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
    }

    file.write(reinterpret_cast<const char*>(&soundEnabled), sizeof(soundEnabled));
    file.write(reinterpret_cast<const char*>(&hideFaces), sizeof(hideFaces));
    file.write(reinterpret_cast<const char*>(&numericCards), sizeof(numericCards));
    file.write(reinterpret_cast<const char*>(&practiceMistakes), sizeof(practiceMistakes));
    file.write(reinterpret_cast<const char*>(&hasPracticeTarget), sizeof(hasPracticeTarget));
    if (hasPracticeTarget) {
        int value = practiceTargetCard.getValue();
        Card::Suit suit = practiceTargetCard.getSuit();
        Card::Rank rank = practiceTargetCard.getRank();
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
        file.write(reinterpret_cast<const char*>(&suit), sizeof(suit));
        file.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
    }
}

void ThreePileCardTrick::loadState(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "读取存档");
    }

    int magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic == SAVE_MAGIC) {
        // 新版存档包含类型信息，可配置玩法还会额外保存牌数。
        std::string type = readString(file);
        if (type != saveType) {
            throw InvalidGameStateException("存档类型与当前魔术不匹配");
        }
        if (configurableSave) {
            int savedDeckSize = 0;
            file.read(reinterpret_cast<char*>(&savedDeckSize), sizeof(savedDeckSize));
            configureDeck(savedDeckSize);
        }
        playerName = readString(file);
    } else {
        // 兼容旧版存档：旧格式没有魔数和玩法类型，只能用于固定牌数玩法。
        if (configurableSave) {
            throw InvalidGameStateException("存档不是可配置魔术存档");
        }

        file.clear();
        file.seekg(0, std::ios::beg);
        size_t nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        if (!file || nameLen > 1024) {
            throw InvalidGameStateException("旧版存档格式无效");
        }
        playerName.resize(nameLen);
        if (nameLen > 0) {
            file.read(&playerName[0], nameLen);
        }
        useAnimation = false;
        magicianMode = false;
    }

    file.read(reinterpret_cast<char*>(&currentRound), sizeof(currentRound));
    file.read(reinterpret_cast<char*>(&score), sizeof(score));
    file.read(reinterpret_cast<char*>(&useColors), sizeof(useColors));
    if (magic == SAVE_MAGIC) {
        file.read(reinterpret_cast<char*>(&useAnimation), sizeof(useAnimation));
        file.read(reinterpret_cast<char*>(&magicianMode), sizeof(magicianMode));
    }

    int savedWorkingDeckSize = 0;
    file.read(reinterpret_cast<char*>(&savedWorkingDeckSize), sizeof(savedWorkingDeckSize));
    if (!file || (savedWorkingDeckSize != 0 && savedWorkingDeckSize != deckSize)) {
        throw InvalidGameStateException("存档中的牌数无效");
    }

    workingDeck.clear();
    // 根据保存的 rank 判断恢复数字牌还是标准扑克牌。
    for (int i = 0; i < savedWorkingDeckSize; ++i) {
        int value = 0;
        Card::Suit suit;
        Card::Rank rank;
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        file.read(reinterpret_cast<char*>(&suit), sizeof(suit));
        file.read(reinterpret_cast<char*>(&rank), sizeof(rank));
        if (!file) {
            throw InvalidGameStateException("读取存档牌面数据失败");
        }

        Card card = (rank == Card::Rank::NUMERIC) ? Card(value) : Card(suit, rank);
        workingDeck.addCard(card);
    }

    if (magic == SAVE_MAGIC) {
        // 后续字段是逐步新增的设置项，读取失败时保留当前默认值以兼容旧存档。
        bool savedSoundEnabled = soundEnabled;
        bool savedHideFaces = hideFaces;
        bool savedNumericCards = numericCards;
        if (file.read(reinterpret_cast<char*>(&savedSoundEnabled), sizeof(savedSoundEnabled))) {
            soundEnabled = savedSoundEnabled;
        }
        if (file.read(reinterpret_cast<char*>(&savedHideFaces), sizeof(savedHideFaces))) {
            hideFaces = savedHideFaces;
        }
        if (file.read(reinterpret_cast<char*>(&savedNumericCards), sizeof(savedNumericCards))) {
            numericCards = savedNumericCards;
        }
        int savedPracticeMistakes = practiceMistakes;
        if (file.read(reinterpret_cast<char*>(&savedPracticeMistakes), sizeof(savedPracticeMistakes))) {
            practiceMistakes = savedPracticeMistakes;
        }
        bool savedHasPracticeTarget = hasPracticeTarget;
        if (file.read(reinterpret_cast<char*>(&savedHasPracticeTarget), sizeof(savedHasPracticeTarget))) {
            hasPracticeTarget = savedHasPracticeTarget;
            if (hasPracticeTarget) {
                int value = 0;
                Card::Suit suit;
                Card::Rank rank;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                file.read(reinterpret_cast<char*>(&suit), sizeof(suit));
                file.read(reinterpret_cast<char*>(&rank), sizeof(rank));
                if (file) {
                    practiceTargetCard = (rank == Card::Rank::NUMERIC) ? Card(value) : Card(suit, rank);
                } else {
                    hasPracticeTarget = false;
                }
            }
        }
        file.clear();
    }

    if (magicianMode && !hasPracticeTarget && workingDeck.getSize() > 0) {
        // 老存档可能没有练习目标牌，加载后补一个有效目标，保证练习模式可继续。
        practiceTargetCard = workingDeck.getCard(0);
        hasPracticeTarget = true;
    }

    hasRevealResult = false;
    lastGuessCorrect = false;
    saveAndExitRequested = false;
    elapsedSeconds = 0;
    clearPiles();
}

void ThreePileCardTrick::setRememberedCard(const Card& card) {
    rememberedCard = card;
}

Card ThreePileCardTrick::getRememberedCard() const {
    return rememberedCard;
}

int ThreePileCardTrick::getDeckSize() const {
    return deckSize;
}
