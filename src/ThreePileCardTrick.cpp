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

void writeString(std::ofstream& file, const std::string& value) {
    size_t length = value.length();
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.c_str(), length);
}

std::string readString(std::ifstream& file) {
    size_t length = 0;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (!file || length > 1024) {
        throw InvalidGameStateException("存档中的字符串长度无效");
    }

    std::string value(length, '\0');
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
    configureDeck(cards);
}

void ThreePileCardTrick::configureDeck(int cards) {
    if (!isDeckSizeAllowed(cards)) {
        throw InvalidInputException("可配置魔术只支持15、21或27张牌");
    }

    deckSize = cards;
    pileSize = cards / 3;
    revealIndex = cards / 2;
    workingDeck = Deck<Card>(deckSize);
    pile1 = Deck<Card>(pileSize);
    pile2 = Deck<Card>(pileSize);
    pile3 = Deck<Card>(pileSize);
}

bool ThreePileCardTrick::isDeckSizeAllowed(int cards) const {
    return cards == 15 || cards == 21 || cards == 27;
}

int ThreePileCardTrick::practiceBaseScore() const {
    return 20 + deckSize / 2;
}

int ThreePileCardTrick::roundPauseMs() const {
    return 800;
}

bool ThreePileCardTrick::shouldShowPracticeHint() const {
    return false;
}

std::string ThreePileCardTrick::failureMessage() const {
    return "哦不！出错了...";
}

PileChoice ThreePileCardTrick::requestAudienceChoice(int) {
    return getUserChoice();
}

bool ThreePileCardTrick::confirmAudienceReveal(const Card&) {
    return Utils::confirm("我猜对了吗？");
}

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
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, workingDeck.getSize() - 1);
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
        for (int value = 1; value <= deckSize; ++value) {
            workingDeck.addCard(Card(value));
        }
        workingDeck.shuffle();
        return;
    }

    Card::Suit suits[] = {Card::Suit::HEARTS, Card::Suit::DIAMONDS, Card::Suit::CLUBS};
    for (int s = 0; s < 3; ++s) {
        for (int r = 1; r <= pileSize; ++r) {
            workingDeck.addCard(Card(suits[s], static_cast<Card::Rank>(r)));
        }
    }

    workingDeck.shuffle();
}

void ThreePileCardTrick::dealIntoPiles() {
    clearPiles();

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
    const int dealDelayMs = 35;

    std::cout << "\n";
    if (useAnimation) {
        Utils::printColored("正在发牌，请观察每张牌的位置...\n", Utils::COLOR_YELLOW);
        std::cout << "\n";
    }

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
        if ((iss >> value) && !(iss >> extra) && value >= 1 && value <= workingDeck.getSize()) {
            return value - 1;
        }

        Utils::printColored("输入超出范围！\n", Utils::COLOR_RED);
        std::cout << "请输入 1 到 " << workingDeck.getSize() << " 之间的数字。\n";
    }
}

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

int ThreePileCardTrick::calculatePracticeScore() const {
    int bonus = practiceBaseScore() - elapsedSeconds / 2 - practiceMistakes * 5;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

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

    dealIntoPiles();
    displayPiles();

    PileChoice choice;
    int chosenPile = 0;
    if (magicianMode) {
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
        saveAndExitRequested = true;
        clearPiles();
        Utils::printColored("\n已准备保存当前进度。\n", Utils::COLOR_GREEN);
        return;
    }

    if (magicianMode) {
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

void ThreePileCardTrick::reveal() {
    if (!isComplete()) {
        throw InvalidGameStateException("三轮完成前不能揭晓");
    }

    Utils::clearScreen();
    Utils::printTitle("揭晓时刻！");
    int selectedIndex = revealIndex;
    if (magicianMode && hasPracticeTarget) {
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

void ThreePileCardTrick::saveState(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "写入存档");
    }

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
