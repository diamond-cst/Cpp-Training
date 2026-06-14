#include "Utils.h"
#include "Exceptions.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <limits>
#include <cstdlib>

// ANSI颜色代码定义 (ANSI color code definitions)
const std::string Utils::COLOR_RESET = "\033[0m";
const std::string Utils::COLOR_RED = "\033[31m";
const std::string Utils::COLOR_GREEN = "\033[32m";
const std::string Utils::COLOR_YELLOW = "\033[33m";
const std::string Utils::COLOR_BLUE = "\033[34m";
const std::string Utils::COLOR_MAGENTA = "\033[35m";
const std::string Utils::COLOR_CYAN = "\033[36m";
const std::string Utils::COLOR_WHITE = "\033[37m";
const std::string Utils::COLOR_BLACK = "\033[30m";

const std::string Utils::BG_RED = "\033[41m";
const std::string Utils::BG_GREEN = "\033[42m";
const std::string Utils::BG_YELLOW = "\033[43m";
const std::string Utils::BG_BLUE = "\033[44m";

const std::string Utils::BOLD = "\033[1m";
const std::string Utils::UNDERLINE = "\033[4m";

// 清屏 (Clear screen)
void Utils::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 暂停指定毫秒 (Pause for milliseconds)
void Utils::sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// 打印带颜色的文本 (Print colored text)
void Utils::printColored(const std::string& text, const std::string& color) {
    std::cout << color << text << COLOR_RESET;
}

// 打印带颜色和样式的文本 (Print colored and styled text)
void Utils::printStyled(const std::string& text,
                       const std::string& color,
                       const std::string& style) {
    std::cout << style << color << text << COLOR_RESET;
}

// 打印分隔线 (Print separator line)
void Utils::printSeparator(char ch, int length) {
    for (int i = 0; i < length; ++i) {
        std::cout << ch;
    }
    std::cout << "\n";
}

// 打印标题 (Print title)
void Utils::printTitle(const std::string& title) {
    int length = 50;
    int padding = (length - title.length()) / 2;

    printSeparator('=', length);
    for (int i = 0; i < padding; ++i) std::cout << " ";
    printStyled(title, COLOR_CYAN, BOLD);
    std::cout << "\n";
    printSeparator('=', length);
}

// 打印带边框的文本 (Print text with border)
void Utils::printBox(const std::string& text) {
    int length = text.length() + 4;

    // 上边框 (Top border)
    std::cout << "╔";
    for (int i = 0; i < length - 2; ++i) std::cout << "═";
    std::cout << "╗\n";

    // 内容 (Content)
    std::cout << "║ " << text << " ║\n";

    // 下边框 (Bottom border)
    std::cout << "╚";
    for (int i = 0; i < length - 2; ++i) std::cout << "═";
    std::cout << "╝\n";
}

// 打印进度条 (Print progress bar)
void Utils::printProgressBar(int current, int total, int width) {
    float progress = static_cast<float>(current) / total;
    int filled = static_cast<int>(progress * width);

    std::cout << "[";
    printColored(std::string(filled, '#'), COLOR_GREEN);
    std::cout << std::string(width - filled, '-');
    std::cout << "] " << static_cast<int>(progress * 100) << "%\r";
    std::cout.flush();

    if (current == total) {
        std::cout << "\n";
    }
}

// 动画效果：逐字打印 (Animation: print character by character)
void Utils::printWithDelay(const std::string& text, int delayMs) {
    for (char ch : text) {
        std::cout << ch << std::flush;
        sleep(delayMs);
    }
    std::cout << "\n";
}

// 动画效果：打点等待 (Animation: waiting dots)
void Utils::printWaitingDots(int count, int delayMs) {
    for (int i = 0; i < count; ++i) {
        std::cout << "." << std::flush;
        sleep(delayMs);
    }
    std::cout << "\n";
}

// 终端提示音 (Terminal beep sound)
void Utils::playBeep(int count, int delayMs) {
    for (int i = 0; i < count; ++i) {
#ifdef __APPLE__
        int result = std::system("afplay /System/Library/Sounds/Glass.aiff >/dev/null 2>&1");
        if (result != 0) {
            std::cout << '\a' << std::flush;
        }
#else
        std::cout << '\a' << std::flush;
#endif
        if (i + 1 < count) {
            sleep(delayMs);
        }
    }
}

void Utils::waitForResume() {
    printStyled("\n游戏已暂停 (Paused)\n", COLOR_YELLOW, BOLD);
    std::cout << "输入 R 恢复游戏。(Enter R to resume.)\n";

    std::string token;
    while (true) {
        std::cout << "暂停中 (Paused)> ";
        std::cin >> token;
        if (std::cin.fail()) {
            if (std::cin.eof()) {
                throw InvalidInputException("Input stream closed during pause");
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (token == "R" || token == "r") {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printColored("游戏继续！(Resumed!)\n", COLOR_GREEN);
            return;
        }
        printColored("请输入 R 恢复。(Please enter R to resume.)\n", COLOR_RED);
    }
}

std::string Utils::hiddenCardFace() {
    return "[??]";
}

// 获取用户输入（带提示）(Get user input with prompt)
std::string Utils::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// 获取整数输入（带验证）(Get integer input with validation)
int Utils::getIntInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail()) {
            if (std::cin.eof()) {
                throw InvalidInputException("Input stream closed while reading an integer");
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printColored("无效输入！请输入数字。(Invalid input! Please enter a number.)\n", COLOR_RED);
        } else if (value < min || value > max) {
            printColored("输入超出范围！(Input out of range!)\n", COLOR_RED);
            std::cout << "请输入 " << min << " 到 " << max << " 之间的数字。\n";
            std::cout << "Please enter a number between " << min << " and " << max << ".\n";
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

// 确认提示 (Confirmation prompt)
bool Utils::confirm(const std::string& message) {
    std::cout << message << " (y/n): ";
    char response;
    std::cin >> response;
    if (std::cin.fail()) {
        if (std::cin.eof()) {
            throw InvalidInputException("Input stream closed while reading confirmation");
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (response == 'y' || response == 'Y');
}

// 按任意键继续 (Press any key to continue)
void Utils::pressAnyKey(const std::string& message) {
    std::cout << "\n" << message << "\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 格式化时间 (Format time)
std::string Utils::formatTime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << "小时 " << minutes << "分钟 " << secs << "秒";
    } else if (minutes > 0) {
        oss << minutes << "分钟 " << secs << "秒";
    } else {
        oss << secs << "秒";
    }

    return oss.str();
}

// 获取当前时间戳 (Get current timestamp)
std::string Utils::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
