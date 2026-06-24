#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#include <thread>

// 工具类
class Utils {
public:
    // ANSI颜色代码
    static const std::string COLOR_RESET;
    static const std::string COLOR_RED;
    static const std::string COLOR_GREEN;
    static const std::string COLOR_YELLOW;
    static const std::string COLOR_BLUE;
    static const std::string COLOR_MAGENTA;
    static const std::string COLOR_CYAN;
    static const std::string COLOR_WHITE;
    static const std::string COLOR_BLACK;

    // 背景颜色
    static const std::string BG_RED;
    static const std::string BG_GREEN;
    static const std::string BG_YELLOW;
    static const std::string BG_BLUE;

    // 文字样式
    static const std::string BOLD; // 粗体
    static const std::string UNDERLINE; // 下划线

    // 清屏
    static void clearScreen();

    // 暂停指定毫秒，适用于动画效果中的延时
    static void sleep(int milliseconds);

    // 打印带颜色的文本
    static void printColored(const std::string& text, const std::string& color);

    // 打印带颜色和样式的文本
    static void printStyled(const std::string& text,
                           const std::string& color,
                           const std::string& style);

    // 打印分隔线
    static void printSeparator(char ch = '=', int length = 50);

    // 打印标题
    static void printTitle(const std::string& title);

    // 打印带边框的文本
    static void printBox(const std::string& text);

    // 打印进度条
    static void printProgressBar(int current, int total, int width = 50);

    // 动画效果：逐字打印
    static void printWithDelay(const std::string& text, int delayMs = 50);

    // 动画效果：打点等待
    static void printWaitingDots(int count = 3, int delayMs = 500);

    // 终端提示音
    static void playBeep(int count = 1, int delayMs = 80);

    // 发牌提示音：轻量、非阻塞，适合逐张发牌
    static void playDealSound();

    // 揭晓提示音：短促同步，紧贴答案显示
    static void playRevealSound();

    // 结果反馈音：短促同步，避免阻塞文字显示
    static void playResultSound(bool success);

    // P/R暂停恢复
    static void waitForResume();

    // 隐藏牌面显示
    static std::string hiddenCardFace();

    // 获取用户输入（带提示）
    static std::string getInput(const std::string& prompt);

    // 获取整数输入（带验证）
    static int getIntInput(const std::string& prompt, int min, int max);

    // 确认提示
    static bool confirm(const std::string& message);

    // 按任意键继续
    static void pressAnyKey(const std::string& message = "按回车键继续...");

    // 格式化时间
    static std::string formatTime(int seconds);

    // 获取当前时间戳
    static std::string getCurrentTimestamp();

private:
    // 私有构造函数，防止实例化
    Utils() = delete;
};

#endif // UTILS_H
