#include "ConfigurableCardTrick.h"
#include "Exceptions.h"
#include "PileChoice.h"
#include "ReplayManager.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
const int SAVE_MAGIC = 0x4D545243; // MTTC
const char SAVE_TYPE[] = "CONFIGURABLE";

void writeString(std::ofstream& file, const std::string& value) {
    size_t length = value.length();
    file.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file.write(value.c_str(), length);
}

std::string readString(std::ifstream& file) {
    size_t length = 0;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (!file || length > 1024) {
        throw InvalidGameStateException("Invalid string length in save file");
    }

    std::string value(length, '\0');
    if (length > 0) {
        file.read(&value[0], length);
    }
    if (!file) {
        throw InvalidGameStateException("Failed to read string from save file");
    }
    return value;
}
}

ConfigurableCardTrick::ConfigurableCardTrick()
    : MagicTrick(21, false), deckSize(21), pileSize(7), revealIndex(10),
      pile1(7), pile2(7), pile3(7) {}

ConfigurableCardTrick::ConfigurableCardTrick(int cards, bool colors, bool animation, bool practiceMode)
    : MagicTrick(cards, colors), deckSize(21), pileSize(7), revealIndex(10),
      pile1(cards / 3), pile2(cards / 3), pile3(cards / 3) {
    useAnimation = animation;
    magicianMode = practiceMode;
    configure(cards);
}

void ConfigurableCardTrick::configure(int cards) {
    if (cards != 15 && cards != 21 && cards != 27) {
        throw InvalidInputException("Configurable trick supports 15, 21, or 27 cards");
    }

    deckSize = cards;
    pileSize = cards / 3;
    revealIndex = cards / 2;
    workingDeck = Deck<Card>(deckSize);
    pile1 = Deck<Card>(pileSize);
    pile2 = Deck<Card>(pileSize);
    pile3 = Deck<Card>(pileSize);
}

void ConfigurableCardTrick::initialize() {
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
    Utils::printTitle(std::to_string(deckSize) + "张牌魔术 (Configurable Card Trick)");
    Utils::printColored("玩家 (Player): ", Utils::COLOR_CYAN);
    std::cout << playerName << "\n";
    Utils::printColored("模式 (Mode): ", Utils::COLOR_CYAN);
    std::cout << (magicianMode ? "魔术师练习 (Magician Practice)" : "观众互动 (Audience)") << "\n\n";

    Utils::printColored("这是" + std::to_string(deckSize) + "张牌:\n", Utils::COLOR_GREEN);
    Utils::printColored("Here are " + std::to_string(deckSize) + " cards:\n\n", Utils::COLOR_GREEN);
    displayState();

    Utils::printColored("\n请在心中记住其中一张牌，但不要告诉我！\n", Utils::COLOR_YELLOW);
    Utils::printColored("Please remember one card in your mind, but don't tell me!\n", Utils::COLOR_YELLOW);
    Utils::pressAnyKey();
}

void ConfigurableCardTrick::initializeCards() {
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

void ConfigurableCardTrick::dealIntoPiles() {
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

void ConfigurableCardTrick::displayPiles() const {
    std::cout << "\n";
    Utils::printColored("牌堆 1 (Pile 1):  ", Utils::COLOR_CYAN);
    for (int i = 0; i < pile1.getSize(); ++i) {
        std::cout << displayCard(pile1.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";

    Utils::printColored("牌堆 2 (Pile 2):  ", Utils::COLOR_CYAN);
    for (int i = 0; i < pile2.getSize(); ++i) {
        std::cout << displayCard(pile2.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";

    Utils::printColored("牌堆 3 (Pile 3):  ", Utils::COLOR_CYAN);
    for (int i = 0; i < pile3.getSize(); ++i) {
        std::cout << displayCard(pile3.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";
}

int ConfigurableCardTrick::getUserChoice() {
    while (true) {
        std::cout << "\n你记住的牌在哪一堆？(输入 1, 2 或 3)\n";
        std::cout << "Which pile contains your card? (Enter 1, 2, 3, or P to pause)\n";
        std::cout << "选择 (Choice): ";

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

void ConfigurableCardTrick::reorganizePiles(int chosenPile) {
    workingDeck.clear();

    Deck<Card>* first = &pile1;
    Deck<Card>* middle = &pile2;
    Deck<Card>* last = &pile3;

    if (chosenPile == 1) {
        first = &pile2;
        middle = &pile1;
        last = &pile3;
    } else if (chosenPile == 3) {
        first = &pile1;
        middle = &pile3;
        last = &pile2;
    }

    for (int i = 0; i < first->getSize(); ++i) {
        workingDeck.addCard((*first)[i]);
    }
    for (int i = 0; i < middle->getSize(); ++i) {
        workingDeck.addCard((*middle)[i]);
    }
    for (int i = 0; i < last->getSize(); ++i) {
        workingDeck.addCard((*last)[i]);
    }
}

void ConfigurableCardTrick::clearPiles() {
    pile1.clear();
    pile2.clear();
    pile3.clear();
}

std::string ConfigurableCardTrick::pileToString(const Deck<Card>& pile) const {
    std::ostringstream oss;
    for (int i = 0; i < pile.getSize(); ++i) {
        if (i > 0) {
            oss << " ";
        }
        oss << pile.getCard(i).toString(false);
    }
    return oss.str();
}

std::string ConfigurableCardTrick::displayCard(const Card& card) const {
    return hideFaces ? Utils::hiddenCardFace() : card.toString(useColors);
}

int ConfigurableCardTrick::calculatePracticeScore() const {
    int base = 20 + deckSize / 2;
    int bonus = base - elapsedSeconds / 2;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

void ConfigurableCardTrick::performRound() {
    if (isComplete()) {
        throw InvalidGameStateException("Trick is already complete");
    }

    ++currentRound;

    Utils::clearScreen();
    Utils::printSeparator('=', 60);
    Utils::printStyled("  第 " + std::to_string(currentRound) + " 轮 (Round " +
                      std::to_string(currentRound) + ")\n", Utils::COLOR_YELLOW, Utils::BOLD);
    Utils::printSeparator('=', 60);

    dealIntoPiles();
    displayPiles();
    if (soundEnabled) {
        Utils::playBeep(1);
    }

    if (magicianMode) {
        Utils::printColored("\n练习提示: 选中的牌堆会被放到中间。\n", Utils::COLOR_MAGENTA);
        Utils::printColored("Practice tip: the selected pile is placed in the middle.\n", Utils::COLOR_MAGENTA);
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
    Utils::printColored("Cards have been reorganized!\n", Utils::COLOR_GREEN);
    if (useAnimation) {
        Utils::sleep(800);
    }
}

bool ConfigurableCardTrick::isComplete() const {
    return currentRound >= 3;
}

void ConfigurableCardTrick::displayState() const {
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        std::cout << displayCard(workingDeck.getCard(i)) << " ";
        if ((i + 1) % pileSize == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

std::string ConfigurableCardTrick::getName() const {
    return std::to_string(deckSize) + " Card Trick (可配置牌数魔术)";
}

void ConfigurableCardTrick::reveal() {
    if (!isComplete()) {
        throw InvalidGameStateException("Cannot reveal before completing all rounds");
    }

    Utils::clearScreen();
    Utils::printTitle("揭晓时刻！(Reveal Time!)");
    Utils::printColored("你记住的牌是第" + std::to_string(revealIndex + 1) + "张牌...\n", Utils::COLOR_YELLOW);
    Utils::printColored("Your card is card #" + std::to_string(revealIndex + 1) + "...\n", Utils::COLOR_YELLOW);

    if (useAnimation) {
        Utils::printWaitingDots(3, 350);
    }
    std::cout << "\n";

    Utils::printStyled("它是: " + workingDeck[revealIndex].toString(useColors) + "\n",
                      Utils::COLOR_GREEN, Utils::BOLD);
    Utils::printStyled("It is: " + workingDeck[revealIndex].toString(useColors) + "\n\n",
                      Utils::COLOR_GREEN, Utils::BOLD);

    bool correct = Utils::confirm("我猜对了吗？(Did I guess correctly?)");
    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    lastGuessCorrect = correct;
    hasRevealResult = true;
    if (correct) {
        int points = magicianMode ? calculatePracticeScore() : 10;
        addScore(points);
        Utils::printColored("\n太好了！魔术成功！\n", Utils::COLOR_GREEN);
        Utils::printColored("Great! Magic succeeded!\n", Utils::COLOR_GREEN);
        Utils::printColored("得分 +" + std::to_string(points) + "! 当前分数: " +
                            std::to_string(score) + "，用时: " +
                            std::to_string(elapsedSeconds) + " 秒\n", Utils::COLOR_YELLOW);
        if (soundEnabled) {
            Utils::playBeep(2);
        }
    } else {
        addScore(-5);
        Utils::printColored("\n练习也会出错，下一次会更稳。\n", Utils::COLOR_RED);
        Utils::printColored("Score -5. Current score: " + std::to_string(score) + "\n",
                            Utils::COLOR_YELLOW);
        if (soundEnabled) {
            Utils::playBeep(1);
        }
    }

    if (replayManager) {
        replayManager->recordReveal(workingDeck[revealIndex].toString(false), correct, score);
    }
}

void ConfigurableCardTrick::saveState(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "open for writing");
    }

    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC), sizeof(SAVE_MAGIC));
    writeString(file, SAVE_TYPE);
    file.write(reinterpret_cast<const char*>(&deckSize), sizeof(deckSize));
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

void ConfigurableCardTrick::loadState(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "open for reading");
    }

    int magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != SAVE_MAGIC) {
        throw InvalidGameStateException("Save file is not a configurable trick save");
    }

    std::string type = readString(file);
    if (type != SAVE_TYPE) {
        throw InvalidGameStateException("Save file type does not match configurable trick");
    }

    int savedDeckSize = 0;
    file.read(reinterpret_cast<char*>(&savedDeckSize), sizeof(savedDeckSize));
    configure(savedDeckSize);

    playerName = readString(file);
    file.read(reinterpret_cast<char*>(&currentRound), sizeof(currentRound));
    file.read(reinterpret_cast<char*>(&score), sizeof(score));
    file.read(reinterpret_cast<char*>(&useColors), sizeof(useColors));
    file.read(reinterpret_cast<char*>(&useAnimation), sizeof(useAnimation));
    file.read(reinterpret_cast<char*>(&magicianMode), sizeof(magicianMode));

    int savedWorkingDeckSize = 0;
    file.read(reinterpret_cast<char*>(&savedWorkingDeckSize), sizeof(savedWorkingDeckSize));
    if (!file || savedWorkingDeckSize != deckSize) {
        throw InvalidGameStateException("Invalid deck size in configurable save");
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
            throw InvalidGameStateException("Failed to read card data from save file");
        }
        Card card = (rank == Card::Rank::NUMERIC) ? Card(value) : Card(suit, rank);
        workingDeck.addCard(card);
    }
}

int ConfigurableCardTrick::getDeckSize() const {
    return deckSize;
}
