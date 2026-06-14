#include "ThreePileCardTrick.h"
#include "Exceptions.h"
#include "PileChoice.h"
#include "ReplayManager.h"
#include "Utils.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
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

void ThreePileCardTrick::initialize() {
    currentRound = 0;
    score = 0;
    hasRevealResult = false;
    lastGuessCorrect = false;
    elapsedSeconds = 0;
    startTime = std::chrono::steady_clock::now();
    workingDeck.clear();
    clearPiles();
    initializeCards();

    Utils::clearScreen();
    Utils::printTitle(getName());
    Utils::printColored("玩家: ", Utils::COLOR_CYAN);
    std::cout << playerName << "\n";
    Utils::printColored("模式: ", Utils::COLOR_CYAN);
    std::cout << (magicianMode ? "魔术师练习" : "观众互动") << "\n\n";

    Utils::printColored("这是" + std::to_string(deckSize) + "张牌:\n", Utils::COLOR_GREEN);
    displayState();

    Utils::printColored("\n请在心中记住其中一张牌，但不要告诉我！\n", Utils::COLOR_YELLOW);
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
            } else if (useAnimation) {
                Utils::sleep(dealDelayMs);
            }
        }
        std::cout << "\n";
    }
}

int ThreePileCardTrick::getUserChoice() {
    while (true) {
        std::cout << "\n你记住的牌在哪一堆？(输入 1, 2 或 3)\n";
        std::cout << "选择: ";

        try {
            PileChoice choice;
            std::cin >> choice;
            if (choice.isPauseRequested()) {
                Utils::waitForResume();
                continue;
            }
            return choice.getValue();
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
    int bonus = practiceBaseScore() - elapsedSeconds / 2;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

void ThreePileCardTrick::performRound() {
    if (isComplete()) {
        throw InvalidGameStateException("魔术已经完成");
    }

    ++currentRound;

    Utils::clearScreen();
    Utils::printSeparator('=', 60);
    Utils::printStyled("  第 " + std::to_string(currentRound) + " 轮\n",
                       Utils::COLOR_YELLOW, Utils::BOLD);
    Utils::printSeparator('=', 60);

    dealIntoPiles();
    displayPiles();

    if (magicianMode && shouldShowPracticeHint()) {
        Utils::printColored("\n练习提示: 选中的牌堆会被放到中间。\n", Utils::COLOR_MAGENTA);
    }

    int choice = getUserChoice();
    if (replayManager) {
        replayManager->recordRound(currentRound,
                                   pileToString(pile1),
                                   pileToString(pile2),
                                   pileToString(pile3),
                                   choice);
    }

    reorganizePiles(choice);

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
    Utils::printColored("你记住的牌是第" + std::to_string(revealIndex + 1) + "张牌...\n",
                        Utils::COLOR_YELLOW);

    if (soundEnabled) {
        Utils::playRevealSound();
    }
    std::cout << "\n";

    Utils::printStyled("它是: " + workingDeck[revealIndex].toString(useColors) + "\n",
                       Utils::COLOR_GREEN, Utils::BOLD);
    std::cout << "\n";

    bool correct = Utils::confirm("我猜对了吗？");
    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    lastGuessCorrect = correct;
    hasRevealResult = true;

    if (correct) {
        int points = magicianMode ? calculatePracticeScore() : 10;
        addScore(points);
        Utils::printColored("\n太好了！魔术成功！\n", Utils::COLOR_GREEN);
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
        replayManager->recordReveal(workingDeck[revealIndex].toString(false), correct, score);
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

    hasRevealResult = false;
    lastGuessCorrect = false;
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
