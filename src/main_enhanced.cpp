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

// 全局排行榜对象
Leaderboard leaderboard("leaderboard.dat");

// 游戏设置结构体
struct GameSettings {
    bool useColor;
    bool useAnimation;
    bool magicianMode;
    bool soundEnabled;
    bool replayEnabled;
    bool hideFaces;
    bool numericCards;
};

// 清理玩家名称，将非字母数字字符替换为下划线
std::string sanitizePlayerName(const std::string& name) {
    std::ostringstream result;
    for (char ch : name) {
        // 将字符转换为无符号字符
        unsigned char uch = static_cast<unsigned char>(ch);
        if ((uch >= '0' && uch <= '9') ||
            (uch >= 'A' && uch <= 'Z') ||
            (uch >= 'a' && uch <= 'z')) {
            result << ch;
        } else if (uch == ' ' || uch == '-' || uch == '_' || uch == '.') {
            result << '_';
        } else {
            // 将非字母数字字符替换为下划线
            result << "_x" << std::uppercase << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(uch) << std::dec;
        }
    }
    // 返回清理后的玩家名称
    std::string value = result.str();
    // 如果玩家名称为空，返回默认值
    return value.empty() ? "player" : value;
}

// 获取玩家存档文件名
std::string playerSaveFilename(const std::string& playerName) {
    return "saves/player_" + sanitizePlayerName(playerName) + ".dat";
}

// 获取最终得分
int getFinalScore(MagicTrick* trick) {
    return trick ? trick->getScore() : 0;
}

// 判断是否有最终结果
bool hasFinalResult(MagicTrick* trick) {
    return trick && trick->hasResult();
}

// 判断最后一次猜测是否正确
bool wasFinalGuessCorrect(MagicTrick* trick) {
    return trick && trick->wasLastGuessCorrect();
}

// 应用高级设置
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

// 附加回放管理器
void attachReplayManager(MagicTrick* trick, ReplayManager* replayManager) {
    if (trick) {
        trick->setReplayManager(replayManager);
    }
}

// 更新排行榜记录
void updateLeaderboardRecord(const std::string& playerName, int finalScore, bool correct, bool hasResult) {
    if (!hasResult) {
        return;
    }

    // 获取玩家记录
    PlayerRecord* existing = leaderboard.getPlayerRecord(playerName);
    // 计算连胜次数
    int nextStreak = correct ? ((existing ? existing->streak : 0) + 1) : 0;
    // 创建玩家记录
    PlayerRecord record(playerName, finalScore, 1, correct ? 1 : 0, nextStreak,
                        Utils::getCurrentTimestamp());
    // 更新排行榜记录
    leaderboard.addOrUpdateRecord(record);

    Utils::printColored("\n排行榜已更新！\n", Utils::COLOR_GREEN);

    // 获取玩家排名
    int rank = leaderboard.getPlayerRank(playerName);
    if (rank > 0 && rank <= 10) {
        Utils::printStyled("你的排名: 第 " + std::to_string(rank) + " 名！\n",
                         Utils::COLOR_YELLOW, Utils::BOLD);
    }
}

// 显示主菜单
void displayMainMenu() {
    Utils::clearScreen();
    Utils::printTitle("21张牌魔术 - 主菜单");
    std::cout << "\n";

    Utils::printColored("1. ", Utils::COLOR_CYAN);
    std::cout << "开始新游戏\n";

    Utils::printColored("2. ", Utils::COLOR_CYAN);
    std::cout << "加载游戏\n";

    Utils::printColored("3. ", Utils::COLOR_CYAN);
    std::cout << "查看排行榜\n";

    Utils::printColored("4. ", Utils::COLOR_CYAN);
    std::cout << "游戏说明\n";

    Utils::printColored("5. ", Utils::COLOR_CYAN);
    std::cout << "查看回放记录\n";

    Utils::printColored("6. ", Utils::COLOR_CYAN);
    std::cout << "网络双人对战\n";

    Utils::printColored("7. ", Utils::COLOR_CYAN);
    std::cout << "退出\n";

    std::cout << "\n";
}

// 选择魔术类型
int selectTrickType() {
    Utils::clearScreen();
    Utils::printTitle("选择魔术类型");
    std::cout << "\n";

    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "21张牌魔术 - 经典版\n";
    std::cout << "   3堆 × 7张，3轮后揭示第11张\n\n";

    Utils::printColored("2. ", Utils::COLOR_YELLOW);
    std::cout << "27张牌魔术 - 进阶版\n";
    std::cout << "   3堆 × 9张，3轮后揭示第14张\n\n";

    Utils::printColored("3. ", Utils::COLOR_MAGENTA);
    std::cout << "可配置牌数魔术 - 高级版\n";
    std::cout << "   支持15/21/27张牌，自动计算揭示位置\n\n";

    return Utils::getIntInput("请选择: ", 1, 3);
}

// 选择牌数
int selectDeckSize() {
    Utils::clearScreen();
    Utils::printTitle("配置牌数");
    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "15张牌\n";
    Utils::printColored("2. ", Utils::COLOR_CYAN);
    std::cout << "21张牌\n";
    Utils::printColored("3. ", Utils::COLOR_YELLOW);
    std::cout << "27张牌\n\n";

    int choice = Utils::getIntInput("请选择: ", 1, 3);
    if (choice == 1) return 15;
    if (choice == 2) return 21;
    return 27;
}

// 收集游戏设置
GameSettings collectGameSettings() {
    GameSettings settings;
    settings.useColor = true;
    settings.useAnimation = true;
    settings.soundEnabled = true;
    settings.replayEnabled = true;

    Utils::printColored("\n已默认开启：彩色显示、动画效果、终端提示音和回放记录。\n",
                        Utils::COLOR_GREEN);

    settings.hideFaces = Utils::confirm("是否隐藏牌面显示？");
    settings.numericCards = Utils::confirm("是否使用数字牌？否则使用标准扑克牌。");

    std::cout << "\n选择游戏模式:\n";
    Utils::printColored("1. ", Utils::COLOR_GREEN);
    std::cout << "观众互动模式 - 猜对+10，猜错-5\n";
    Utils::printColored("2. ", Utils::COLOR_YELLOW);
    std::cout << "魔术师练习模式 - 模拟观众选择，最后手动揭牌\n";
    settings.magicianMode = (Utils::getIntInput("请选择: ", 1, 2) == 2);

    return settings;
}

// 开始新游戏
void startNewGame() {
    try {
        Utils::clearScreen();

        // 获取玩家名称
        std::string playerName = Utils::getInput("请输入你的名字: ");
        if (playerName.empty()) {
            playerName = "玩家";
        }

        GameSettings settings = collectGameSettings();

        // 选择魔术类型
        int trickType = selectTrickType();

        // 创建魔术对象
        std::unique_ptr<MagicTrick> trick;

        // 根据魔术类型创建魔术对象
        if (trickType == 1) {
            trick = std::make_unique<TwentyOneCardTrick>(settings.useColor);
        } else if (trickType == 2) {
            trick = std::make_unique<TwentySevenCardTrick>(settings.useColor);
        } else {
            int deckSize = selectDeckSize();
            trick = std::make_unique<ConfigurableCardTrick>(
                deckSize, settings.useColor, settings.useAnimation, settings.magicianMode);
        }

        // 设置玩家名称
        trick->setPlayerName(playerName);
        // 应用高级设置
        applyAdvancedSettings(trick.get(), settings);

        // 创建回放管理器
        ReplayManager replayManager("replays");
        // 如果回放记录启用，则启动回放管理器
        if (settings.replayEnabled) {
            std::string modeName = settings.magicianMode
                ? "魔术师练习"
                : "观众互动";
            replayManager.start(playerName, trick->getName(), modeName);
            attachReplayManager(trick.get(), &replayManager);
        }

        // 初始化
        trick->initialize();

        // 执行三轮
        while (!trick->isComplete()) {
            // 执行一轮
            trick->performRound();
            // 如果请求保存并退出，则保存游戏进度
            if (trick->shouldSaveAndExit()) {
                // 保存游戏进度
                std::string filename = playerSaveFilename(playerName);
                trick->saveState(filename);
                Utils::printColored("游戏进度已保存到玩家存档: " + filename + "\n",
                                    Utils::COLOR_GREEN);
                // 等待用户按任意键继续
                Utils::pressAnyKey();
                // 退出游戏
                return;
            }
        }

        // 揭示结果
        trick->reveal();

        // 更新排行榜记录
        updateLeaderboardRecord(playerName,
                                getFinalScore(trick.get()),
                                wasFinalGuessCorrect(trick.get()),
                                hasFinalResult(trick.get()));
        // 如果回放记录启用，则保存回放记录
        if (settings.replayEnabled) {
            // 保存回放记录
            replayManager.save();
            // 导出HTML回放报告
            replayManager.exportHtml();
            Utils::printColored("\n回放已保存: " + replayManager.getTextFilename() + "\n",
                                Utils::COLOR_GREEN);
            Utils::printColored("HTML报告已导出: " + replayManager.getHtmlFilename() + "\n",
                                Utils::COLOR_GREEN);
        }

        // 询问是否保存
        std::cout << "\n";
        if (Utils::confirm("是否保存游戏？")) {
            // 获取玩家存档文件名
            std::string filename = playerSaveFilename(playerName);
            // 保存游戏进度
            trick->saveState(filename);
            Utils::printColored("游戏已保存到玩家存档: " + filename + "\n", Utils::COLOR_GREEN);
        }

        // 等待用户按任意键继续
        Utils::pressAnyKey();

    } catch (const MagicTrickException& e) {
        // 打印错误信息
        Utils::printColored("\n错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::bad_alloc& e) {
        Utils::printColored("\n内存分配失败: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::ios_base::failure& e) {
        Utils::printColored("\n文件流错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::exception& e) {
        Utils::printColored("\n未知错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 加载游戏
void loadGame() {
    try {
        Utils::clearScreen();
        Utils::printTitle("加载游戏");

        std::string playerName = Utils::getInput("\n输入玩家名读取存档: ");
        if (playerName.empty()) {
            playerName = "玩家";
        }
        std::string filename = playerSaveFilename(playerName);
        if (!std::ifstream(filename)) {
            throw FileIOException(filename, "读取存档");
        }

        // 尝试加载不同类型的存档
        std::unique_ptr<MagicTrick> trick = std::make_unique<TwentyOneCardTrick>();
        // 尝试加载存档
        try {
            trick->loadState(filename);
        } catch (...) {
            trick = std::make_unique<TwentySevenCardTrick>();
            // 尝试加载存档
            try {
                trick->loadState(filename);
            } catch (...) {
                // 尝试加载可配置牌数的存档
                trick = std::make_unique<ConfigurableCardTrick>();
                trick->loadState(filename);
            }
        }
        // 设置动画效果
        trick->setUseAnimation(true);
        // 设置声音效果
        trick->setSoundEnabled(true);

        Utils::printColored("\n游戏已加载！\n", Utils::COLOR_GREEN);
        Utils::printColored("玩家: ", Utils::COLOR_CYAN);
        std::cout << trick->getPlayerName() << "\n";
        Utils::printColored("魔术类型: ", Utils::COLOR_CYAN);
        std::cout << trick->getName() << "\n";
        Utils::printColored("当前轮数: ", Utils::COLOR_CYAN);
        std::cout << trick->getCurrentRound() << "\n";

        // 等待用户按任意键继续
        Utils::pressAnyKey();

        // 创建回放管理器
        ReplayManager replayManager("replays");
        // 启动回放管理器
        replayManager.start(trick->getPlayerName(), trick->getName(), "加载继续");
        attachReplayManager(trick.get(), &replayManager);

        // 继续游戏
        while (!trick->isComplete()) {
            // 执行一轮
            trick->performRound();
            // 如果请求保存并退出，则保存游戏进度
            if (trick->shouldSaveAndExit()) {
                std::string filename = playerSaveFilename(trick->getPlayerName());
                trick->saveState(filename);
                Utils::printColored("游戏进度已保存到玩家存档: " + filename + "\n",
                                    Utils::COLOR_GREEN);
                // 等待用户按任意键继续
                Utils::pressAnyKey();
                return;
            }
        }

        // 揭示结果
        trick->reveal();
        // 更新排行榜记录
        updateLeaderboardRecord(trick->getPlayerName(),
                                getFinalScore(trick.get()),
                                wasFinalGuessCorrect(trick.get()),
                                hasFinalResult(trick.get()));
        // 保存回放记录
        replayManager.save();
        // 导出HTML回放报告
        replayManager.exportHtml();
        Utils::printColored("\n继续游戏回放已保存: " + replayManager.getTextFilename() + "\n",
                            Utils::COLOR_GREEN);

        // 等待用户按任意键继续
        Utils::pressAnyKey();

    } catch (const FileIOException& e) {
        Utils::printColored("\n文件错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::printColored("存档不存在或已损坏。\n",
                          Utils::COLOR_YELLOW);
        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::bad_alloc& e) {
        Utils::printColored("\n内存分配失败: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::ios_base::failure& e) {
        Utils::printColored("\n文件流错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 查看排行榜
void viewLeaderboard() {
    Utils::clearScreen();
    leaderboard.displayColored();
    Utils::pressAnyKey();
}

// 查看回放记录
void viewReplays() {
    try {
        Utils::clearScreen();
        Utils::printTitle("回放记录");

        // 列出回放记录
        auto files = ReplayManager::listReplayFiles("replays");
        // 如果回放记录为空，则提示并返回
        if (files.empty()) {
            Utils::printColored("\n暂无回放记录。\n", Utils::COLOR_YELLOW);
            Utils::pressAnyKey();
            return;
        }

        // 计算最大显示数量
        int maxDisplay = std::min(10, static_cast<int>(files.size()));
        // 循环显示回放记录
        for (int i = 0; i < maxDisplay; ++i) {
            Utils::printColored(std::to_string(i + 1) + ". ", Utils::COLOR_CYAN);
            std::cout << files[i] << "\n";
        }

        // 选择要查看的回放
        int choice = Utils::getIntInput("\n请选择要查看的回放: ", 1, maxDisplay);
        Utils::clearScreen();
        Utils::printTitle("回放内容");
        ReplayManager::displayReplayFile(files[choice - 1]);
        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n回放读取失败: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 获取网络端口输入
int getNetworkPortInput(const std::string& prompt, int defaultPort) {
    while (true) {
        // 获取输入
        std::string input = Utils::getInput(prompt);
        if (input.empty()) {
            // 如果输入为空，则返回默认端口
            return defaultPort;
        }

        // 解析输入
        std::istringstream iss(input);
        int port = 0;
        char extra = '\0';
        if ((iss >> port) && !(iss >> extra) && port >= 1024 && port <= 65535) {
            // 如果端口在范围内，则返回端口
            return port;
        }

        Utils::printColored("端口必须是 1024 到 65535 之间的数字。\n", Utils::COLOR_RED);
    }
}

// 获取网络主机输入
std::string getNetworkHostInput(const std::string& prompt, const std::string& defaultHost) {
    // 获取输入
    std::string host = Utils::getInput(prompt);
    // 如果输入为空，则返回默认主机
    return host.empty() ? defaultHost : host;
}

// 网络双人对战
void startNetworkDuel() {
    try {
        // 默认端口
        const int defaultPort = 5000;
        // 默认主机
        const std::string defaultHost = "127.0.0.1";

        // 清屏
        Utils::clearScreen();
        // 打印标题
        Utils::printTitle("双人魔术模式");

        // 打印选项
        Utils::printColored("1. ", Utils::COLOR_GREEN);
        std::cout << "我是魔术师：创建房间并等待观众\n";
        Utils::printColored("2. ", Utils::COLOR_YELLOW);
        std::cout << "我是观众：进入魔术师房间\n\n";

        // 选择角色
        int role = Utils::getIntInput("请选择角色: ", 1, 2);

        if (role == 1) {
            // 获取房间端口
            int port = getNetworkPortInput("房间端口（回车默认 " +
                                           std::to_string(defaultPort) + "）: ",
                                           defaultPort);
            // 打印提示
            Utils::printColored("\n请让观众进入本机 127.0.0.1，房间端口 " +
                                std::to_string(port) + "。\n",
                                Utils::COLOR_CYAN);
            // 运行魔术师服务器
            NetworkGame::runMagicianServer(port);
        } else {
            std::string host = getNetworkHostInput("魔术师地址（回车默认本机）: ",
                                                   defaultHost);
            int port = getNetworkPortInput("房间端口（回车默认 " +
                                           std::to_string(defaultPort) + "）: ",
                                           defaultPort);
            NetworkGame::runAudienceClient(host, port);
        }

        Utils::pressAnyKey();
    } catch (const MagicTrickException& e) {
        Utils::printColored("\n网络对战错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    } catch (const std::exception& e) {
        Utils::printColored("\n网络对战未知错误: ", Utils::COLOR_RED);
        std::cerr << e.what() << "\n";
        Utils::pressAnyKey();
    }
}

// 显示游戏说明
void showInstructions() {
    Utils::clearScreen();
    Utils::printTitle("游戏说明");
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
    std::cout << "• 魔术师练习模式：系统模拟观众选堆，最后由魔术师选择揭晓牌\n";
    std::cout << "• 练习模式评分会综合用时和错误次数\n";
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

// 主函数
int main() {
    Utils::printStyled("\n正在启动21张牌魔术...\n", Utils::COLOR_CYAN, Utils::BOLD);
    Utils::sleep(1000);

    bool running = true;

    // 循环显示主菜单
    while (running) {
        displayMainMenu();

        // 获取用户选择
        int choice = Utils::getIntInput("请选择: ", 1, 7);

        // 根据用户选择执行相应操作
        switch (choice) {
            // 开始新游戏
            case 1:
                startNewGame();
                break;

            // 加载游戏
            case 2:
                loadGame();
                break;

            // 查看排行榜
            case 3:
                viewLeaderboard();
                break;

            // 显示游戏说明
            case 4:
                showInstructions();
                break;

            // 查看回放记录
            case 5:
                viewReplays();
                break;

            // 网络双人对战
            case 6:
                startNetworkDuel();
                break;

            // 退出
            case 7:
                Utils::clearScreen();
                Utils::printStyled("\n感谢游玩！再见！\n", Utils::COLOR_GREEN, Utils::BOLD);
                // 停止运行
                running = false;
                break;

            // 无效选择
            default:
                Utils::printColored("\n无效选择！请重试。\n", Utils::COLOR_RED);
                Utils::pressAnyKey();
                break;
        }
    }

    return 0;
}
