#include "Card.h"
#include "Deck.h"
#include "ConfigurableCardTrick.h"
#include "TwentyOneCardTrick.h"
#include "Exceptions.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

// 测试Card类 (Test Card class)
void testCard() {
    std::cout << "测试卡牌类...\n";

    // 测试标准扑克牌 (Test standard playing card)
    Card card1(Card::Suit::HEARTS, Card::Rank::ACE);
    Card card2(Card::Suit::SPADES, Card::Rank::KING);

    assert(card1.getSuit() == Card::Suit::HEARTS);
    assert(card1.getRank() == Card::Rank::ACE);
    assert(card1.getValue() == 1);

    // 测试比较运算符 (Test comparison operators)
    assert(card1 < card2);
    assert(card2 > card1);
    assert(card1 != card2);

    // 测试拷贝 (Test copy)
    Card card3 = card1;
    assert(card3 == card1);

    std::cout << "  牌1: " << card1.toString(false) << "\n";
    std::cout << "  牌2: " << card2.toString(false) << "\n";
    std::cout << "  卡牌类测试通过！\n\n";
}

// 测试Deck类 (Test Deck class)
void testDeck() {
    std::cout << "测试牌组类...\n";

    Deck<Card> deck(10);

    // 添加卡牌 (Add cards)
    for (int i = 1; i <= 5; ++i) {
        deck.addCard(Card(i));
    }

    assert(deck.getSize() == 5);
    assert(!deck.isEmpty());

    // 测试下标访问 (Test subscript access)
    Card firstCard = deck[0];
    assert(firstCard.getValue() == 1);

    // 测试拷贝 (Test copy)
    Deck<Card> deck2 = deck;
    assert(deck2.getSize() == deck.getSize());
    assert(deck2 == deck);

    // 测试合并运算符 (Test merge operator)
    Deck<Card> deck3 = deck + deck2;
    assert(deck3.getSize() == 10);

    // 测试移除 (Test remove)
    deck.removeCard(0);
    assert(deck.getSize() == 4);

    std::cout << "  牌组大小: " << deck.getSize() << "\n";
    std::cout << "  牌组类测试通过！\n\n";
}

// 测试异常处理 (Test exception handling)
void testExceptions() {
    std::cout << "测试异常处理...\n";

    try {
        // 测试越界异常 (Test out of bounds)
        Deck<Card> deck(5);
        deck.addCard(Card(1));
        Card card = deck[10]; // 应该抛出异常
        assert(false); // 不应该到达这里
    } catch (const OutOfBoundsException& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }

    try {
        // 测试无效卡牌 (Test invalid card)
        Card card(-5); // 应该抛出异常
        assert(false);
    } catch (const InvalidCardException& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }

    std::cout << "  异常处理测试通过！\n\n";
}

// 测试魔术算法 (Test magic trick algorithm)
void testMagicTrick() {
    std::cout << "测试21张牌魔术算法...\n";

    TwentyOneCardTrick trick(false);
    trick.setPlayerName("测试玩家");

    // 初始化 (Initialize)
    std::cout << "  初始化魔术...\n";
    // 注意：这里不调用initialize()因为它需要用户输入

    std::cout << "  魔术名称: " << trick.getName() << "\n";
    std::cout << "  玩家名称: " << trick.getPlayerName() << "\n";
    std::cout << "  当前轮数: " << trick.getCurrentRound() << "\n";
    std::cout << "  是否完成: " << (trick.isComplete() ? "是" : "否") << "\n";

    std::cout << "  魔术类测试通过！\n\n";
}

int findPileContainingCard(const std::vector<std::vector<std::string>>& piles,
                           const std::string& card) {
    for (int pileIndex = 0; pileIndex < static_cast<int>(piles.size()); ++pileIndex) {
        for (const auto& value : piles[pileIndex]) {
            if (value == card) {
                return pileIndex + 1;
            }
        }
    }
    return 0;
}

void testGuiStateMachine() {
    std::cout << "测试GUI状态机接口...\n";

    TwentyOneCardTrick trick(false);
    trick.setPlayerName("GUI测试玩家");
    trick.initializeForGui();
    std::string target = trick.getWorkingDeckCardsForGui(true).front();

    for (int round = 0; round < 3; ++round) {
        trick.dealCurrentRoundForGui();
        auto piles = trick.getCurrentPilesForGui(true);
        int pile = findPileContainingCard(piles, target);
        assert(pile >= 1 && pile <= 3);
        trick.applyPileChoiceForGui(pile);
    }

    assert(trick.isComplete());
    assert(trick.getWorkingDeckCardsForGui(true)[trick.getRevealIndexForGui()] == target);
    bool correct = trick.finalizeRevealForGui(trick.getRevealIndexForGui(), true);
    assert(correct);
    assert(trick.hasResult());
    assert(trick.getScore() == 10);

    ConfigurableCardTrick configurable(27, false, false, false);
    configurable.initializeForGui();
    assert(configurable.getDeckSize() == 27);
    assert(configurable.getRevealIndexForGui() == 13);

    std::cout << "  GUI状态机接口测试通过！\n\n";
}

// 测试保存/加载 (Test save/load)
void testSaveLoad() {
    std::cout << "测试保存/加载功能...\n";

    try {
        TwentyOneCardTrick trick1(false);
        trick1.setPlayerName("测试玩家");

        // 保存 (Save)
        std::string filename = "saves/test_save.dat";
        trick1.saveState(filename);
        std::cout << "  游戏已保存到: " << filename << "\n";

        // 加载 (Load)
        TwentyOneCardTrick trick2(false);
        trick2.loadState(filename);
        std::cout << "  游戏已加载\n";

        assert(trick2.getPlayerName() == "测试玩家");
        assert(trick2.getCurrentRound() == trick1.getCurrentRound());

        std::cout << "  保存/加载测试通过！\n\n";
    } catch (const FileIOException& e) {
        std::cerr << "  文件IO错误: " << e.what() << "\n";
    }
}

// 主函数 (Main function)
int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  21张牌魔术 - 单元测试\n";
    std::cout << "========================================\n\n";

    try {
        testCard();
        testDeck();
        testExceptions();
        testMagicTrick();
        testGuiStateMachine();
        testSaveLoad();

        std::cout << "========================================\n";
        std::cout << "  所有测试通过！\n";
        std::cout << "========================================\n\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n测试失败: " << e.what() << "\n";
        return 1;
    }
}
