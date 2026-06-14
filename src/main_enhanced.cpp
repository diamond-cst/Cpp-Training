#include "TwentyOneCardTrick.h"
#include "TwentySevenCardTrick.h"
#include "ConfigurableCardTrick.h"
#include "Leaderboard.h"
#include "NetworkGame.h"
#include "ReplayManager.h"
#include "Utils.h"
#include "Exceptions.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

// 全局排行榜对象 (Global leaderboard object)
Leaderboard leaderboard("leaderboard.dat");

struct GameSettings {
    bool useColor;
    bool useAnimation;
    bool magicianMode;
    bool soundEnabled;
    bool replayEnabled;
    bool hideFaces;
    bool numericCards;
};

std::string sanitizePlayerName(const std::string& name) {
    std::ostringstream result;
    for (char ch : name) {
        unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch)) {
            result << ch;
        } else {
            result << "_" << std::uppercase << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(uch) << std::dec;
        }
    }
    std::string value = result.str();
    return value.empty() ? "Player" : value;
}

std::string playerSaveFilename(const std::string& playerName) {
    return "saves/player_" + sanitizePlayerName(playerName) + ".dat";
}

int getFinalScore(MagicTrick* trick) {
    return trick ? trick->getScore() : 0;
}

bool hasFinalResult(MagicTrick* trick) {
    return trick && trick->hasResult();
}

bool wasFinalGuessCorrect(MagicTrick* trick) {
    return trick && trick->wasLastGuessCorrect();
}

void applyAdvancedSettings(MagicTrick* trick, const GameSettings& settings) {
    if (!trick) {
        return;
    }
    trick->setUseAnimation(settings.useAnimation);
    trick->setMagicianMode(settings.magicianMode);
    trick->setSoundEnabled(settings.soundEnabled);
    trick->setHideFaces(settings.hideFaces);
    trick->setNumericCards(settings.numericCards);
}

void attachReplayManager(MagicTrick* trick, ReplayManager* replayManager) {
    if (trick) {
        trick->setReplayManager(replayManager);
    }
}

void updateLeaderboardRecord(const std::string& playerName, int finalScore, bool correct, bool hasResult) {
    if (!hasResult) {
        return;
    }

    PlayerRecord* existing = leaderboard.getPlayerRecord(playerName);
    int nextStreak = correct ? ((existing ? existing->streak : 0) + 1) : 0;
    PlayerRecord record(playerName, finalScore, 1, correct ? 1 : 0, nextStreak,
                        Utils::getCurrentTimestamp());
    leaderboard.addOrUpdateRecord(record);

    Utils::printColored("\n排行榜已更新！(Leaderboard updated!)\n", Utils::COLOR_GREEN);

    int rank = leaderboard.getPlayerRank(playerName);
    if (rank > 0 && rank <= 10) {
        Utils::printStyled("你的排名: 第 " + std::to_string(rank) + " 名！\n",
                         Utils::COLOR_YELLOW, Utils::BOLD);
        Utils::printStyled("Your rank: #" + std::to_string(rank) + "!\n",
                         Utils::COLOR_YELLOW, Utils::BOLD);
    }
}

// 显示主菜单 (Display main menu)
void displayMainMenu() {
    Utils::clearScreen();
    Utils::printTitle("21张牌魔术 - 主菜单");
    Utils::printTitle("21 Card Trick - Main Menu");
    std::cout << "\n";

    Utils::printColored("1. ", Utils::COLOR_CYAN);
    std::cout << "开始新游戏 (Start New Game)\n";

    Utils::printColored("2. ", Utils::COLOR_CYAN);
    std::cout << "加载游戏 (Load Game)\n";

    Utils::printColored("3. ", Utils::COLOR_CYAN);
    std::cout << "查看排行榜 (View Leaderboard)\n";

    Utils::printColored("4. ", Utils::COLOR_CYAN);
    std::cout << "游戏说明 (Instructions)\n";

    Utils::printColored("5. ", Utils::COLOR_CYAN);
    std::cout << "查看回放记录 (View Replays)\n";

    Utils::printColored("6. ", Utils::COLOR_CYAN);
    std::cout << "网络双人对战 (Network Duel)\n";

    Utils::printColored("7. ", Utils::COLOR_CYAN);
    std::cout << "退出 (Exit)\n";

    std::cout << "\n";
}

// 选择魔术类型 (Select trick type)
int selectTrickType() {
    Utils::clearScreen();
    Utils::printTitle("选择魔术类型 (Select Trick Type)");
    std::cout << "\n";

    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "21张牌魔术 (21 Card Trick) - 经典版\n";
    std::cout << "   3堆 × 7张，3轮后揭示第11张\n\n";

    Utils::printColored("2. ", Utils::COLOR_YELLOW);
    std::cout << "27张牌魔术 (27 Card Trick) - 进阶版\n";
    std::cout << "   3堆 × 9张，3轮后揭示第14张\n\n";

    Utils::printColored("3. ", Utils::COLOR_MAGENTA);
    std::cout << "可配置牌数魔术 (Configurable Trick) - 高级版\n";
    std::cout << "   支持15/21/27张牌，自动计算揭示位置\n\n";

    return Utils::getIntInput("请选择 (Choose): ", 1, 3);
}

int selectDeckSize() {
    Utils::clearScreen();
    Utils::printTitle("配置牌数 (Configure Deck Size)");
    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "15张牌 (15 cards)\n";
    Utils::printColored("2. ", Utils::COLOR_CYAN);
    std::cout << "21张牌 (21 cards)\n";
    Utils::printColored("3. ", Utils::COLOR_YELLOW);
    std::cout << "27张牌 (27 cards)\n\n";

    int choice = Utils::getIntInput("请选择 (Choose): ", 1, 3);
    if (choice == 1) return 15;
    if (choice == 2) return 21;
    return 27;
}

GameSettings collectGameSettings() {
    GameSettings settings;
    settings.useColor = Utils::confirm("是否使用彩色显示？(Use colors?)");
    settings.useAnimation = Utils::confirm("是否启用动画效果？(Enable animation effects?)");
    settings.soundEnabled = Utils::confirm("是否启用终端提示音？(Enable terminal sound?)");
    settings.replayEnabled = Utils::confirm("是否记录本局回放？(Record replay?)");
    settings.hideFaces = Utils::confirm("是否隐藏牌面显示？(Hide card faces?)");
    settings.numericCards = Utils::confirm("是否使用数字牌？否则使用标准扑克牌。(Use numeric cards?)");

    std::cout << "\n选择游戏模式 (Select game mode):\n";
    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "观众互动模式 (Audience Mode) - 猜对+10，猜错-5\n";
    Utils::printColored("2. ", Utils::COLOR_YELLOW);
    std::cout << "魔术师练习模式 (Magician Practice) - 根据完成时间评分，含练习提示\n";
    settings.magicianMode = (Utils::getIntInput("请选择 (Choose): ", 1, 2) == 2);

    return settings;
}

// 开始新游戏 (Start new game)
void startNewGame() {
    try {
        Utils::clearScreen();

        // 获取玩家名称 (Get player name)
        std::string playerName = Utils::getInput("请输入你的名字 (Enter your name): ");
        if (playerName.empty()) {
            playerName = "Player";
        }

        GameSettings settings = collectGameSettings();

        // 选择魔术类型 (Select trick type)
        int trickType = selectTrickType();

        // 创建魔术对象 (Create trick object)
        std::unique_ptr<MagicTrick> trick;

        if (trickType == 1) {
            trick = std::make_unique<TwentyOneCardTrick>(settings.useColor);
        } else if (trickType == 2) {
            trick = std::make_unique<TwentySevenCardTrick>(settings.useColor);
        } else {
            int deckSize = selectDeckSize();
            trick = std::make_unique<ConfigurableCardTrick>(
                deckSize, settings.useColor, settings.useAnimation, settings.magicianMode);
        }

        trick->setPlayerName(playerName);
        applyAdvancedSettings(trick.get(), settings);

        ReplayManager replayManager("replays");
        if (settings.replayEnabled) {
            std::string modeName = settings.magicianMode
                ? "魔术师练习 (Magician Practice)"
                : "观众互动 (Audience)";
            replayManager.start(playerName, trick->getName(), modeName);
            attachReplayManager(trick.get(), &replayManager);
        }

        // 初始化 (Initialize)
        trick->initialize();

        // 执行三轮 (Perform three rounds)
        while (!trick->isComplete()) {
            trick->performRound();
        }

        // 揭示结果 (Reveal result)
        trick->reveal();

        updateLeaderboardRecord(playerName,
                                getFinalScore(trick.get()),
                                wasFinalGuessCorrect(trick.get()),
                                hasFinalResult(trick.get()));
        if (settings.replayEnabled) {
            replayManager.save();
            replayManager.exportHtml();
            Utils::printColored("\n回放已保存 (Replay saved): " + replayManager.getTextFilename() + "\n",
                                Utils::COLOR_GREEN);
            Utils::printColored("HTML报告已导出 (HTML report exported): " + replayManager.getHtmlFilename() + "\n",
                                Utils::COLOR_GREEN);
        }

        // 询问是否保存 (Ask to save)
        std::cout << "\n";
        if (Utils::confirm("是否保存游戏？(Save game?)")) {
            std::string filename = playerSaveFilename(playerName);
            trick->saveState(filename);
            Utils::printColored("游戏已保存到玩家存档: " + filename + "\n", Utils::COLOR_GREEN);
        }

        Utils::pressAnyKey();

    } catch (const MagicTrickException& e) {
        Utils::printColored("\n错误 (Error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::bad_alloc& e) {
        Utils::printColored("\n内存分配失败 (Memory allocation failed): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::ios_base::failure& e) {
        Utils::printColored("\n文件流错误 (File stream error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::exception& e) {
        Utils::printColored("\n未知错误 (Unknown error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 加载游戏 (Load game)
void loadGame() {
    try {
        Utils::clearScreen();
        Utils::printTitle("加载游戏 (Load Game)");

        std::string playerName = Utils::getInput("\n输入玩家名读取存档 (Enter player name): ");
        if (playerName.empty()) {
            playerName = "Player";
        }
        std::string filename = playerSaveFilename(playerName);
        if (!std::ifstream(filename)) {
            throw FileIOException(filename, "open for reading");
        }

        // 尝试加载不同类型的存档 (Try loading different trick saves)
        std::unique_ptr<MagicTrick> trick = std::make_unique<TwentyOneCardTrick>();

        try {
            trick->loadState(filename);
        } catch (...) {
            trick = std::make_unique<TwentySevenCardTrick>();
            try {
                trick->loadState(filename);
            } catch (...) {
                trick = std::make_unique<ConfigurableCardTrick>();
                trick->loadState(filename);
            }
        }

        Utils::printColored("\n游戏已加载！(Game loaded!)\n", Utils::COLOR_GREEN);
        Utils::printColored("玩家 (Player): ", Utils::COLOR_CYAN);
        std::cout << trick->getPlayerName() << "\n";
        Utils::printColored("魔术类型 (Trick type): ", Utils::COLOR_CYAN);
        std::cout << trick->getName() << "\n";
        Utils::printColored("当前轮数 (Current round): ", Utils::COLOR_CYAN);
        std::cout << trick->getCurrentRound() << "\n";

        Utils::pressAnyKey();

        ReplayManager replayManager("replays");
        replayManager.start(trick->getPlayerName(), trick->getName(), "加载继续 (Loaded Game)");
        attachReplayManager(trick.get(), &replayManager);

        // 继续游戏 (Continue game)
        while (!trick->isComplete()) {
            trick->performRound();
        }

        // 揭示结果 (Reveal result)
        trick->reveal();
        updateLeaderboardRecord(trick->getPlayerName(),
                                getFinalScore(trick.get()),
                                wasFinalGuessCorrect(trick.get()),
                                hasFinalResult(trick.get()));
        replayManager.save();
        replayManager.exportHtml();
        Utils::printColored("\n继续游戏回放已保存 (Replay saved): " + replayManager.getTextFilename() + "\n",
                            Utils::COLOR_GREEN);

        Utils::pressAnyKey();

    } catch (const FileIOException& e) {
        Utils::printColored("\n文件错误 (File error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::printColored("存档不存在或已损坏。(Save file doesn't exist or is corrupted.)\n",
                          Utils::COLOR_YELLOW);
        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n错误 (Error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::bad_alloc& e) {
        Utils::printColored("\n内存分配失败 (Memory allocation failed): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::ios_base::failure& e) {
        Utils::printColored("\n文件流错误 (File stream error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 查看排行榜 (View leaderboard)
void viewLeaderboard() {
    Utils::clearScreen();
    leaderboard.displayColored();
    Utils::pressAnyKey();
}

// 查看回放记录 (View replay records)
void viewReplays() {
    try {
        Utils::clearScreen();
        Utils::printTitle("回放记录 (Replay Records)");

        auto files = ReplayManager::listReplayFiles("replays");
        if (files.empty()) {
            Utils::printColored("\n暂无回放记录。(No replay records.)\n", Utils::COLOR_YELLOW);
            Utils::pressAnyKey();
            return;
        }

        int maxDisplay = std::min(10, static_cast<int>(files.size()));
        for (int i = 0; i < maxDisplay; ++i) {
            Utils::printColored(std::to_string(i + 1) + ". ", Utils::COLOR_CYAN);
            std::cout << files[i] << "\n";
        }

        int choice = Utils::getIntInput("\n请选择要查看的回放 (Choose replay): ", 1, maxDisplay);
        Utils::clearScreen();
        Utils::printTitle("回放内容 (Replay Content)");
        ReplayManager::displayReplayFile(files[choice - 1]);
        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n回放读取失败 (Replay read failed): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 网络双人对战 (Network two-player duel)
void startNetworkDuel() {
    try {
        Utils::clearScreen();
        Utils::printTitle("网络双人对战 (Network Duel)");

        Utils::printColored("1. ", Utils::COLOR_GREEN);
        std::cout << "魔术师端：创建房间并等待观众连接\n";
        Utils::printColored("2. ", Utils::COLOR_YELLOW);
        std::cout << "观众端：连接魔术师房间\n\n";

        int role = Utils::getIntInput("请选择角色 (Choose role): ", 1, 2);
        int port = Utils::getIntInput("请输入端口 1024-65535 (Port): ", 1024, 65535);

        if (role == 1) {
            NetworkGame::runMagicianServer(port);
        } else {
            std::string host = Utils::getInput("请输入魔术师IP，例如127.0.0.1 (Host IP): ");
            NetworkGame::runAudienceClient(host, port);
        }

        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n网络对战错误 (Network duel error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::exception& e) {
        Utils::printColored("\n网络对战未知错误 (Unknown network error): ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 显示游戏说明 (Display instructions)
void showInstructions() {
    Utils::clearScreen();
    Utils::printTitle("游戏说明 (Instructions)");
    std::cout << "\n";

    Utils::printStyled("21张牌魔术原理：\n", Utils::COLOR_CYAN, Utils::BOLD);
    std::cout << "1. 从21张牌中记住一张\n";
    std::cout << "2. 将牌分成3堆（每堆7张）\n";
    std::cout << "3. 告诉我你的牌在哪一堆\n";
    std::cout << "4. 我会将选中的堆放在中间重新整理\n";
    std::cout << "5. 重复3次后，你的牌必定在第11张！\n\n";

    Utils::printStyled("27张牌魔术原理：\n", Utils::COLOR_YELLOW, Utils::BOLD);
    std::cout << "1. 从27张牌中记住一张\n";
    std::cout << "2. 将牌分成3堆（每堆9张）\n";
    std::cout << "3. 告诉我你的牌在哪一堆\n";
    std::cout << "4. 我会将选中的堆放在中间重新整理\n";
    std::cout << "5. 重复3次后，你的牌必定在第14张！\n\n";

    Utils::printStyled("计分规则：\n", Utils::COLOR_GREEN, Utils::BOLD);
    std::cout << "• 观众模式猜对：+10分\n";
    std::cout << "• 魔术师练习模式猜对：按完成时间评分\n";
    std::cout << "• 猜错：-5分\n";
    std::cout << "• 排行榜记录前10名玩家\n\n";

    Utils::printStyled("高级配置：\n", Utils::COLOR_MAGENTA, Utils::BOLD);
    std::cout << "• 可配置牌数支持15、21、27张\n";
    std::cout << "• 可选择数字牌或标准扑克牌\n";
    std::cout << "• 可开启隐藏牌面模式\n";
    std::cout << "• 彩色显示会按扑克牌花色着色\n";
    std::cout << "• 动画效果会逐张发牌显示\n\n";

    Utils::printStyled("任务4加分功能：\n", Utils::COLOR_CYAN, Utils::BOLD);
    std::cout << "• 支持终端提示音\n";
    std::cout << "• 支持游戏回放记录\n";
    std::cout << "• 每局可导出HTML回放报告\n\n";

    Utils::printStyled("网络双人对战：\n", Utils::COLOR_YELLOW, Utils::BOLD);
    std::cout << "• 魔术师端创建房间，观众端输入IP和端口连接\n";
    std::cout << "• 魔术师端负责发牌、整理牌堆和揭示答案\n";
    std::cout << "• 观众端通过网络提交牌堆选择\n\n";

    Utils::printStyled("Qt图形界面：\n", Utils::COLOR_CYAN, Utils::BOLD);
    std::cout << "• 使用 make gui 编译独立Qt界面\n";
    std::cout << "• 图形界面支持鼠标点击选择牌堆\n\n";

    Utils::pressAnyKey();
}

// 主函数 (Main function)
int main() {
    Utils::printStyled("\n正在启动21张牌魔术...\n", Utils::COLOR_CYAN, Utils::BOLD);
    Utils::printStyled("Starting 21 Card Trick...\n\n", Utils::COLOR_CYAN, Utils::BOLD);
    Utils::sleep(1000);

    bool running = true;

    while (running) {
        displayMainMenu();

        int choice = Utils::getIntInput("请选择 (Choose): ", 1, 7);

        switch (choice) {
            case 1:
                startNewGame();
                break;

            case 2:
                loadGame();
                break;

            case 3:
                viewLeaderboard();
                break;

            case 4:
                showInstructions();
                break;

            case 5:
                viewReplays();
                break;

            case 6:
                startNetworkDuel();
                break;

            case 7:
                Utils::clearScreen();
                Utils::printStyled("\n感谢游玩！再见！\n", Utils::COLOR_GREEN, Utils::BOLD);
                Utils::printStyled("Thanks for playing! Goodbye!\n\n", Utils::COLOR_GREEN, Utils::BOLD);
                running = false;
                break;

            default:
                Utils::printColored("\n无效选择！请重试。\n", Utils::COLOR_RED);
                Utils::printColored("Invalid choice! Please try again.\n", Utils::COLOR_RED);
                Utils::pressAnyKey();
                break;
        }
    }

    return 0;
}
