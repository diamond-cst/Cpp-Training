#include "TwentyOneCardTrick.h"
#include "Exceptions.h"
#include "PileChoice.h"
#include "ReplayManager.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <limits>

namespace {
const int SAVE_MAGIC = 0x4D545243; // MTTC
const char SAVE_TYPE[] = "TWENTY_ONE";

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
TwentyOneCardTrick::TwentyOneCardTrick()
    : MagicTrick(21, false), pile1(7), pile2(7), pile3(7) {}

TwentyOneCardTrick::TwentyOneCardTrick(bool colors)
    : MagicTrick(21, colors), pile1(7), pile2(7), pile3(7) {}

// 初始化魔术 (Initialize trick)
void TwentyOneCardTrick::initialize() {
    currentRound = 0;
    score = 0;
    hasRevealResult = false;
    lastGuessCorrect = false;
    elapsedSeconds = 0;
    startTime = std::chrono::steady_clock::now();
    workingDeck.clear();
    clearPiles();
    initializeCards();

    std::cout << "\n========================================\n";
    std::cout << "  欢迎来到21张牌魔术！\n";
    std::cout << "  Welcome to 21 Card Trick!\n";
    std::cout << "========================================\n\n";

    std::cout << "玩家: " << playerName << "\n\n";

    std::cout << "这是21张牌:\n";
    std::cout << "Here are 21 cards:\n\n";
    displayState();

    std::cout << "\n请在心中记住其中一张牌，但不要告诉我！\n";
    std::cout << "Please remember one card in your mind, but don't tell me!\n";
    std::cout << "\n按回车键继续...\n";
    std::cout << "Press Enter to continue...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// 初始化21张标准扑克牌 (Initialize 21 standard playing cards)
void TwentyOneCardTrick::initializeCards() {
    // 创建21张不同的扑克牌 (Create 21 different playing cards)
    // 使用红桃A-7, 方块A-7, 梅花A-7 (Hearts A-7, Diamonds A-7, Clubs A-7)

    if (numericCards) {
        for (int value = 1; value <= 21; ++value) {
            workingDeck.addCard(Card(value));
        }
        workingDeck.shuffle();
        return;
    }

    Card::Suit suits[] = {Card::Suit::HEARTS, Card::Suit::DIAMONDS, Card::Suit::CLUBS};
    for (int s = 0; s < 3; ++s) {
        for (int r = 1; r <= 7; ++r) {
            Card card(suits[s], static_cast<Card::Rank>(r));
            workingDeck.addCard(card);
        }
    }

    // 洗牌 (Shuffle)
    workingDeck.shuffle();
}

// 发牌到三堆 (Deal cards into three piles)
void TwentyOneCardTrick::dealIntoPiles() {
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
void TwentyOneCardTrick::displayPiles() const {
    std::cout << "\n";
    std::cout << "牌堆 1 (Pile 1):  ";
    for (int i = 0; i < pile1.getSize(); ++i) {
        std::cout << displayCard(pile1.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";

    std::cout << "牌堆 2 (Pile 2):  ";
    for (int i = 0; i < pile2.getSize(); ++i) {
        std::cout << displayCard(pile2.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";

    std::cout << "牌堆 3 (Pile 3):  ";
    for (int i = 0; i < pile3.getSize(); ++i) {
        std::cout << displayCard(pile3.getCard(i)) << " ";
        if (useAnimation) Utils::sleep(80);
    }
    std::cout << "\n";
}

// 获取用户选择 (Get user choice)
int TwentyOneCardTrick::getUserChoice() {
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
void TwentyOneCardTrick::reorganizePiles(int chosenPile) {
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
void TwentyOneCardTrick::clearPiles() {
    pile1.clear();
    pile2.clear();
    pile3.clear();
}

std::string TwentyOneCardTrick::pileToString(const Deck<Card>& pile) const {
    std::ostringstream oss;
    for (int i = 0; i < pile.getSize(); ++i) {
        if (i > 0) {
            oss << " ";
        }
        oss << pile.getCard(i).toString(false);
    }
    return oss.str();
}

std::string TwentyOneCardTrick::displayCard(const Card& card) const {
    return hideFaces ? Utils::hiddenCardFace() : card.toString(useColors);
}

int TwentyOneCardTrick::calculatePracticeScore() const {
    int bonus = 30 - elapsedSeconds / 2;
    if (bonus < 5) {
        bonus = 5;
    }
    return bonus;
}

// 执行一轮 (Perform one round)
void TwentyOneCardTrick::performRound() {
    if (isComplete()) {
        throw InvalidGameStateException("Trick is already complete");
    }

    currentRound++;

    std::cout << "\n========================================\n";
    std::cout << "  第 " << currentRound << " 轮 (Round " << currentRound << ")\n";
    std::cout << "========================================\n";

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

    std::cout << "\n牌已重新整理！\n";
    std::cout << "Cards have been reorganized!\n";
    if (useAnimation) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

// 检查是否完成 (Check if complete)
bool TwentyOneCardTrick::isComplete() const {
    return currentRound >= 3;
}

// 显示当前状态 (Display current state)
void TwentyOneCardTrick::displayState() const {
    for (int i = 0; i < workingDeck.getSize(); ++i) {
        std::cout << displayCard(workingDeck.getCard(i)) << " ";
        if ((i + 1) % 7 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

// 获取魔术名称 (Get trick name)
std::string TwentyOneCardTrick::getName() const {
    return "21 Card Trick (21张牌魔术)";
}

// 揭示结果 (Reveal result)
void TwentyOneCardTrick::reveal() {
    if (!isComplete()) {
        throw InvalidGameStateException("Cannot reveal before completing all rounds");
    }

    std::cout << "\n========================================\n";
    std::cout << "  揭晓时刻！(Reveal Time!)\n";
    std::cout << "========================================\n\n";

    std::cout << "你记住的牌是第11张牌...\n";
    std::cout << "Your card is the 11th card...\n\n";

    std::cout << "它是: " << workingDeck[10].toString(useColors) << "\n";
    std::cout << "It is: " << workingDeck[10].toString(useColors) << "\n\n";

    std::cout << "我猜对了吗？(Did I guess correctly?) (y/n): ";
    char answer;
    std::cin >> answer;

    elapsedSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime).count());
    bool correct = (answer == 'y' || answer == 'Y');
    lastGuessCorrect = correct;
    hasRevealResult = true;
    if (correct) {
        int points = magicianMode ? calculatePracticeScore() : 10;
        std::cout << "\n太好了！魔术成功！\n";
        std::cout << "Great! Magic succeeded!\n";
        addScore(points);
        std::cout << "得分 +" << points << "! 当前分数: " << score
                  << "，用时: " << elapsedSeconds << " 秒\n";
        std::cout << "Score +" << points << "! Current score: " << score
                  << ", time: " << elapsedSeconds << " seconds\n";
        if (soundEnabled) {
            Utils::playBeep(2);
        }
    } else {
        std::cout << "\n哦不！出错了...\n";
        std::cout << "Oh no! Something went wrong...\n";
        addScore(-5);
        std::cout << "得分 -5. 当前分数: " << score << "\n";
        std::cout << "Score -5. Current score: " << score << "\n";
        if (soundEnabled) {
            Utils::playBeep(1);
        }
    }

    if (replayManager) {
        replayManager->recordReveal(workingDeck[10].toString(false), correct, score);
    }
}

// 保存状态 (Save state)
void TwentyOneCardTrick::saveState(const std::string& filename) const {
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
void TwentyOneCardTrick::loadState(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw FileIOException(filename, "open for reading");
    }

    int magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic == SAVE_MAGIC) {
        std::string type = readString(file);
        if (type != SAVE_TYPE) {
            throw InvalidGameStateException("Save file type does not match 21 card trick");
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
    if (!file || (deckSize != 0 && deckSize != 21)) {
        throw InvalidGameStateException("Save file does not contain a 21-card deck");
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
void TwentyOneCardTrick::setRememberedCard(const Card& card) {
    rememberedCard = card;
}

// 获取记住的牌 (Get remembered card)
Card TwentyOneCardTrick::getRememberedCard() const {
    return rememberedCard;
}
