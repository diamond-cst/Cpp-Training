#include "TwentySevenCardTrick.h"
#include "Exceptions.h"
#include "PileChoice.h"
#include "ReplayManager.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {
const int SAVE_MAGIC = 0x4D545243; // MTTC
const char SAVE_TYPE[] = "TWENTY_SEVEN";

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

// 构造函数 (Constructor)
TwentySevenCardTrick::TwentySevenCardTrick()
    : MagicTrick(27, false), pile1(9), pile2(9), pile3(9) {}

TwentySevenCardTrick::TwentySevenCardTrick(bool colors)
    : MagicTrick(27, colors), pile1(9), pile2(9), pile3(9) {}

// 初始化魔术 (Initialize trick)
void TwentySevenCardTrick::initialize() {
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
    Utils::printTitle("27张牌魔术 (27 Card Trick)");
    std::cout << "\n";

    Utils::printColored("玩家: ", Utils::COLOR_CYAN);
    std::cout << playerName << "\n\n";

    Utils::printColored("这是27张牌:\n", Utils::COLOR_GREEN);
    Utils::printColored("Here are 27 cards:\n\n", Utils::COLOR_GREEN);
    displayState();

    std::cout << "\n";
    Utils::printColored("请在心中记住其中一张牌，但不要告诉我！\n", Utils::COLOR_YELLOW);
    Utils::printColored("Please remember one card in your mind, but don't tell me!\n", Utils::COLOR_YELLOW);

    Utils::pressAnyKey();
}

// 初始化27张标准扑克牌 (Initialize 27 standard playing cards)
void TwentySevenCardTrick::initializeCards() {
    // 创建27张不同的扑克牌 (Create 27 different playing cards)
    // 使用红桃A-9, 方块A-9, 梅花A-9 (Hearts A-9, Diamonds A-9, Clubs A-9)

    if (numericCards) {
        for (int value = 1; value <= 27; ++value) {
            workingDeck.addCard(Card(value));
        }
        workingDeck.shuffle();
        return;
    }

    Card::Suit suits[] = {Card::Suit::HEARTS, Card::Suit::DIAMONDS, Card::Suit::CLUBS};
    for (int s = 0; s < 3; ++s) {
        for (int r = 1; r <= 9; ++r) {
            Card card(suits[s], static_cast<Card::Rank>(r));
            workingDeck.addCard(card);
        }
    }

    // 洗牌 (Shuffle)
    workingDeck.shuffle();
}

// 发牌到三堆 (Deal cards into three piles)
void TwentySevenCardTrick::dealIntoPiles() {
    clearPiles();

    // 轮流发牌：第一堆、第二堆、第三堆 (Deal in round-robin: pile1, pile2, pile3)
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

// 显示三个牌堆 (Display three piles)
void TwentySevenCardTrick::displayPiles() const {
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

// 获取用户选择 (Get user choice)
int TwentySevenCardTrick::getUserChoice() {
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

// 重新整理牌堆 (Reorganize piles)
void TwentySevenCardTrick::reorganizePiles(int chosenPile) {
    workingDeck.clear();

    // 将选中的牌堆放在中间 (Place chosen pile in the middle)
    if (chosenPile == 1) {
        // 顺序：pile2, pile1, pile3
        for (int i = 0; i < pile2.getSize(); ++i) {
            workingDeck.addCard(pile2[i]);
        }
        for (int i = 0; i < pile1.getSize(); ++i) {
            workingDeck.addCard(pile1[i]);
        }
        for (int i = 0; i < pile3.getSize(); ++i) {
            workingDeck.addCard(pile3[i]);
        }
    } else if (chosenPile == 2) {
        // 顺序：pile1, pile2, pile3
        for (int i = 0; i < pile1.getSize(); ++i) {
            workingDeck.addCard(pile1[i]);
        }
        for (int i = 0; i < pile2.getSize(); ++i) {
            workingDeck.addCard(pile2[i]);
        }
        for (int i = 0; i < pile3.getSize(); ++i) {
            workingDeck.addCard(pile3[i]);
        }
    } else { // chosenPile == 3
        // 顺序：pile1, pile3, pile2
        for (int i = 0; i < pile1.getSize(); ++i) {
            workingDeck.addCard(pile1[i]);
        }
        for (int i = 0; i < pile3.getSize(); ++i) {
            workingDeck.addCard(pile3[i]);
        }
        for (int i = 0; i < pile2.getSize(); ++i) {
            workingDeck.addCard(pile2[i]);
        }
    }
}

// 清空牌堆 (Clear piles)
void TwentySevenCardTrick::clearPiles() {
    pile1.clear();
    pile2.clear();
    pile3.clear();
}

std::string TwentySevenCardTrick::pileToString(const Deck<Card>& pile) const {
    std::ostringstream oss;
    for (int i = 0; i < pile.getSize(); ++i) {
        if (i > 0) {
            oss << " ";
        }
        oss << pile.getCard(i).toString(false);
    }
    return oss.str();
}

std::string TwentySevenCardTrick::displayCard(const Card& card) const {
    return hideFaces ? Utils::hiddenCardFace() : card.toString(useColors);
}

int TwentySevenCardTrick::calculatePracticeScore() const {
    int bonus = 35 - elapsedSeconds / 2;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

// 执行一轮 (Perform one round)
void TwentySevenCardTrick::performRound() {
    if (isComplete()) {
        throw InvalidGameStateException("Trick is already complete");
    }

    currentRound++;

    Utils::clearScreen();
    Utils::printSeparator('=', 60);
    Utils::printStyled("  第 " + std::to_string(currentRound) + " 轮 (Round " +
                      std::to_string(currentRound) + ")\n", Utils::COLOR_YELLOW, Utils::BOLD);
    Utils::printSeparator('=', 60);

    // 发牌到三堆 (Deal into three piles)
    dealIntoPiles();

    // 显示三个牌堆 (Display three piles)
    displayPiles();
    if (soundEnabled) {
        Utils::playBeep(1);
    }

    // 获取用户选择 (Get user choice)
    int choice = getUserChoice();
    if (replayManager) {
        replayManager->recordRound(currentRound,
                                   pileToString(pile1),
                                   pileToString(pile2),
                                   pileToString(pile3),
                                   choice);
    }

    // 重新整理牌堆 (Reorganize piles)
    reorganizePiles(choice);

    Utils::printColored("\n牌已重新整理！\n", Utils::COLOR_GREEN);
    Utils::printColored("Cards have been reorganized!\n", Utils::COLOR_GREEN);

    if (useAnimation) {
        Utils::sleep(1000);
    }
}

// 检查是否完成 (Check if complete)
bool TwentySevenCardTrick::isComplete() const {
    return currentRound >= 3;
}

// 显示当前状态 (Display current state)
void TwentySevenCardTrick::displayState() const {
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        std::cout << displayCard(workingDeck.getCard(i)) << " ";
        if ((i + 1) % 9 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

// 获取魔术名称 (Get trick name)
std::string TwentySevenCardTrick::getName() const {
    return "27 Card Trick (27张牌魔术)";
}

// 揭示结果 (Reveal result)
void TwentySevenCardTrick::reveal() {
    if (!isComplete()) {
        throw InvalidGameStateException("Cannot reveal before completing all rounds");
    }

    Utils::clearScreen();
    Utils::printTitle("揭晓时刻！(Reveal Time!)");
    std::cout << "\n";

    Utils::printColored("你记住的牌是第14张牌...\n", Utils::COLOR_YELLOW);
    Utils::printColored("Your card is the 14th card...\n", Utils::COLOR_YELLOW);

    if (useAnimation) {
        Utils::printWaitingDots(3, 500);
    }
    std::cout << "\n";

    Utils::printStyled("它是: " + workingDeck[13].toString(useColors) + "\n",
                      Utils::COLOR_GREEN, Utils::BOLD);
    Utils::printStyled("It is: " + workingDeck[13].toString(useColors) + "\n\n",
                      Utils::COLOR_GREEN, Utils::BOLD);

    bool correct = Utils::confirm("我猜对了吗？(Did I guess correctly?)");
    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    lastGuessCorrect = correct;
    hasRevealResult = true;

    if (correct) {
        int points = magicianMode ? calculatePracticeScore() : 10;
        Utils::printColored("\n太好了！魔术成功！\n", Utils::COLOR_GREEN);
        Utils::printColored("Great! Magic succeeded!\n", Utils::COLOR_GREEN);
        addScore(points);
        Utils::printColored("得分 +" + std::to_string(points) + "! 当前分数: " +
                            std::to_string(score) + "，用时: " +
                            std::to_string(elapsedSeconds) + " 秒\n", Utils::COLOR_YELLOW);
        Utils::printColored("Score +" + std::to_string(points) + "! Current score: " +
                            std::to_string(score) + ", time: " +
                            std::to_string(elapsedSeconds) + " seconds\n", Utils::COLOR_YELLOW);
        if (soundEnabled) {
            Utils::playBeep(2);
        }
    } else {
        Utils::printColored("\n哦不！出错了...\n", Utils::COLOR_RED);
        Utils::printColored("Oh no! Something went wrong...\n", Utils::COLOR_RED);
        addScore(-5);
        Utils::printColored("得分 -5. 当前分数: " + std::to_string(score) + "\n", Utils::COLOR_YELLOW);
        Utils::printColored("Score -5. Current score: " + std::to_string(score) + "\n", Utils::COLOR_YELLOW);
        if (soundEnabled) {
            Utils::playBeep(1);
        }
    }

    if (replayManager) {
        replayManager->recordReveal(workingDeck[13].toString(false), correct, score);
    }
}

// 保存状态 (Save state)
void TwentySevenCardTrick::saveState(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "open for writing");
    }

    file.write(reinterpret_cast<const char*>(&SAVE_MAGIC), sizeof(SAVE_MAGIC));
    writeString(file, SAVE_TYPE);
    writeString(file, playerName);

    // 保存当前轮数和分数 (Save current round and score)
    file.write(reinterpret_cast<const char*>(&currentRound), sizeof(currentRound));
    file.write(reinterpret_cast<const char*>(&score), sizeof(score));
    file.write(reinterpret_cast<const char*>(&useColors), sizeof(useColors));
    file.write(reinterpret_cast<const char*>(&useAnimation), sizeof(useAnimation));
    file.write(reinterpret_cast<const char*>(&magicianMode), sizeof(magicianMode));

    // 保存牌堆大小和内容 (Save deck size and content)
    int deckSize = workingDeck.getSize();
    file.write(reinterpret_cast<const char*>(&deckSize), sizeof(deckSize));
    for (int i = 0; i < deckSize; ++i) {
        Card card = workingDeck.getCard(i);
        int value = card.getValue();
        Card::Suit suit = card.getSuit();
        Card::Rank rank = card.getRank();
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
        file.write(reinterpret_cast<const char*>(&suit), sizeof(suit));
        file.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
    }

    file.close();
}

// 加载状态 (Load state)
void TwentySevenCardTrick::loadState(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "open for reading");
    }

    int magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic == SAVE_MAGIC) {
        std::string type = readString(file);
        if (type != SAVE_TYPE) {
            throw InvalidGameStateException("Save file type does not match 27 card trick");
        }
        playerName = readString(file);
    } else {
        file.clear();
        file.seekg(0, std::ios::beg);
        size_t nameLen;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        if (!file || nameLen > 1024) {
            throw InvalidGameStateException("Invalid legacy save file");
        }
        playerName.resize(nameLen);
        if (nameLen > 0) {
            file.read(&playerName[0], nameLen);
        }
        useAnimation = false;
        magicianMode = false;
    }

    // 加载当前轮数和分数 (Load current round and score)
    file.read(reinterpret_cast<char*>(&currentRound), sizeof(currentRound));
    file.read(reinterpret_cast<char*>(&score), sizeof(score));
    file.read(reinterpret_cast<char*>(&useColors), sizeof(useColors));
    if (magic == SAVE_MAGIC) {
        file.read(reinterpret_cast<char*>(&useAnimation), sizeof(useAnimation));
        file.read(reinterpret_cast<char*>(&magicianMode), sizeof(magicianMode));
    }

    // 加载牌堆 (Load deck)
    int deckSize;
    file.read(reinterpret_cast<char*>(&deckSize), sizeof(deckSize));
    if (!file || (deckSize != 0 && deckSize != 27)) {
        throw InvalidGameStateException("Save file does not contain a 27-card deck");
    }
    workingDeck.clear();
    for (int i = 0; i < deckSize; ++i) {
        int value;
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

    file.close();
}

// 设置记住的牌 (Set remembered card)
void TwentySevenCardTrick::setRememberedCard(const Card& card) {
    rememberedCard = card;
}

// 获取记住的牌 (Get remembered card)
Card TwentySevenCardTrick::getRememberedCard() const {
    return rememberedCard;
}
