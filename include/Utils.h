#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#include <thread>

// 工具类 (Utility class)
class Utils {
public:
    // ANSI颜色代码 (ANSI color codes)
    static const std::string COLOR_RESET;
    static const std::string COLOR_RED;
    static const std::string COLOR_GREEN;
    static const std::string COLOR_YELLOW;
    static const std::string COLOR_BLUE;
    static const std::string COLOR_MAGENTA;
    static const std::string COLOR_CYAN;
    static const std::string COLOR_WHITE;
    static const std::string COLOR_BLACK;

    // 背景颜色 (Background colors)
    static const std::string BG_RED;
    static const std::string BG_GREEN;
    static const std::string BG_YELLOW;
    static const std::string BG_BLUE;

    // 文字样式 (Text styles)
    static const std::string BOLD;
    static const std::string UNDERLINE;

    // 清屏 (Clear screen)
    static void clearScreen();

    // 暂停指定毫秒 (Pause for milliseconds)
    static void sleep(int milliseconds);

    // 打印带颜色的文本 (Print colored text)
    static void printColored(const std::string& text, const std::string& color);

    // 打印带颜色和样式的文本 (Print colored and styled text)
    static void printStyled(const std::string& text,
                           const std::string& color,
                           const std::string& style);

    // 打印分隔线 (Print separator line)
    static void printSeparator(char ch = '=', int length = 50);

    // 打印标题 (Print title)
    static void printTitle(const std::string& title);

    // 打印带边框的文本 (Print text with border)
    static void printBox(const std::string& text);

    // 打印进度条 (Print progress bar)
    static void printProgressBar(int current, int total, int width = 50);

    // 动画效果：逐字打印 (Animation: print character by character)
    static void printWithDelay(const std::string& text, int delayMs = 50);

    // 动画效果：打点等待 (Animation: waiting dots)
    static void printWaitingDots(int count = 3, int delayMs = 500);

    // 终端提示音 (Terminal beep sound)
    static void playBeep(int count = 1, int delayMs = 80);

    // P/R暂停恢复 (P/R pause and resume)
    static void waitForResume();

    // 隐藏牌面显示 (Hidden card face text)
    static std::string hiddenCardFace();

    // 获取用户输入（带提示）(Get user input with prompt)
    static std::string getInput(const std::string& prompt);

    // 获取整数输入（带验证）(Get integer input with validation)
    static int getIntInput(const std::string& prompt, int min, int max);

    // 确认提示 (Confirmation prompt)
    static bool confirm(const std::string& message);

    // 按任意键继续 (Press any key to continue)
    static void pressAnyKey(const std::string& message = "按回车键继续... (Press Enter to continue...)");

    // 格式化时间 (Format time)
    static std::string formatTime(int seconds);

    // 获取当前时间戳 (Get current timestamp)
    static std::string getCurrentTimestamp();

private:
    // 私有构造函数，防止实例化 (Private constructor to prevent instantiation)
    Utils() = delete;
};

#endif // UTILS_H
