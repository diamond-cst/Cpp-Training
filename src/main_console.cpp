#include "TwentyOneCardTrick.h"
#include "Exceptions.h"
#include <iostream>
#include <memory>

// 清屏函数 (Clear screen function)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 显示主菜单 (Display main menu)
void displayMainMenu() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║     21张牌魔术 - 主菜单                ║\n";
    std::cout << "║     21 Card Trick - Main Menu          ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "1. 开始新游戏 (Start New Game)\n";
    std::cout << "2. 加载游戏 (Load Game)\n";
    std::cout << "3. 查看排行榜 (View Leaderboard)\n";
    std::cout << "4. 设置 (Settings)\n";
    std::cout << "5. 退出 (Exit)\n";
    std::cout << "\n";
    std::cout << "请选择 (Choose): ";
}

// 开始新游戏 (Start new game)
void startNewGame() {
    try {
        clearScreen();

        std::cout << "请输入你的名字 (Enter your name): ";
        std::string playerName;
        std::cin.ignore();
        std::getline(std::cin, playerName);

        std::cout << "是否使用彩色显示？(Use colors?) (y/n): ";
        char useColor;
        std::cin >> useColor;

        bool colors = (useColor == 'y' || useColor == 'Y');

        // 创建魔术对象 (Create trick object)
        std::unique_ptr<MagicTrick> trick = std::make_unique<TwentyOneCardTrick>(colors);
        trick->setPlayerName(playerName);

        // 初始化 (Initialize)
        trick->initialize();

        // 执行三轮 (Perform three rounds)
        while (!trick->isComplete()) {
            trick->performRound();
        }

        // 揭示结果 (Reveal result)
        trick->reveal();

        std::cout << "\n是否保存游戏？(Save game?) (y/n): ";
        char saveChoice;
        std::cin >> saveChoice;

        if (saveChoice == 'y' || saveChoice == 'Y') {
            std::cout << "选择存档槽位 (1-5) (Choose save slot 1-5): ";
            int slot;
            std::cin >> slot;

            if (slot >= 1 && slot <= 5) {
                std::string filename = "saves/slot_" + std::to_string(slot) + ".dat";
                trick->saveState(filename);
                std::cout << "游戏已保存！(Game saved!)\n";
            } else {
                std::cout << "无效的槽位！(Invalid slot!)\n";
            }
        }

        std::cout << "\n按回车键返回主菜单...\n";
        std::cout << "Press Enter to return to main menu...\n";
        std::cin.ignore();
        std::cin.get();

    } catch (const MagicTrickException& e) {
        std::cerr << "错误 (Error): " << e.what() << "\n";
        std::cout << "\n按回车键继续...\n";
        std::cin.ignore();
        std::cin.get();
    } catch (const std::exception& e) {
        std::cerr << "未知错误 (Unknown error): " << e.what() << "\n";
        std::cout << "\n按回车键继续...\n";
        std::cin.ignore();
        std::cin.get();
    }
}

// 加载游戏 (Load game)
void loadGame() {
    try {
        clearScreen();

        std::cout << "选择存档槽位 (1-5) (Choose save slot 1-5): ";
        int slot;
        std::cin >> slot;

        if (slot < 1 || slot > 5) {
            std::cout << "无效的槽位！(Invalid slot!)\n";
            std::cout << "\n按回车键继续...\n";
            std::cin.ignore();
            std::cin.get();
            return;
        }

        std::string filename = "saves/slot_" + std::to_string(slot) + ".dat";

        std::unique_ptr<MagicTrick> trick = std::make_unique<TwentyOneCardTrick>();
        trick->loadState(filename);

        std::cout << "\n游戏已加载！(Game loaded!)\n";
        std::cout << "玩家 (Player): " << trick->getPlayerName() << "\n";
        std::cout << "当前轮数 (Current round): " << trick->getCurrentRound() << "\n";

        // 继续游戏 (Continue game)
        while (!trick->isComplete()) {
            trick->performRound();
        }

        // 揭示结果 (Reveal result)
        trick->reveal();

        std::cout << "\n按回车键返回主菜单...\n";
        std::cout << "Press Enter to return to main menu...\n";
        std::cin.ignore();
        std::cin.get();

    } catch (const FileIOException& e) {
        std::cerr << "文件错误 (File error): " << e.what() << "\n";
        std::cout << "存档不存在或已损坏。(Save file doesn't exist or is corrupted.)\n";
        std::cout << "\n按回车键继续...\n";
        std::cin.ignore();
        std::cin.get();
    } catch (const MagicTrickException& e) {
        std::cerr << "错误 (Error): " << e.what() << "\n";
        std::cout << "\n按回车键继续...\n";
        std::cin.ignore();
        std::cin.get();
    }
}

// 主函数 (Main function)
int main() {
    std::cout << "正在启动21张牌魔术...\n";
    std::cout << "Starting 21 Card Trick...\n\n";

    bool running = true;

    while (running) {
        clearScreen();
        displayMainMenu();

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                startNewGame();
                break;

            case 2:
                loadGame();
                break;

            case 3:
                clearScreen();
                std::cout << "排行榜功能即将推出...\n";
                std::cout << "Leaderboard feature coming soon...\n";
                std::cout << "\n按回车键继续...\n";
                std::cin.ignore();
                std::cin.get();
                break;

            case 4:
                clearScreen();
                std::cout << "设置功能即将推出...\n";
                std::cout << "Settings feature coming soon...\n";
                std::cout << "\n按回车键继续...\n";
                std::cin.ignore();
                std::cin.get();
                break;

            case 5:
                std::cout << "\n感谢游玩！再见！\n";
                std::cout << "Thanks for playing! Goodbye!\n";
                running = false;
                break;

            default:
                std::cout << "\n无效选择！请重试。\n";
                std::cout << "Invalid choice! Please try again.\n";
                std::cout << "\n按回车键继续...\n";
                std::cin.ignore();
                std::cin.get();
                break;
        }
    }

    return 0;
}
