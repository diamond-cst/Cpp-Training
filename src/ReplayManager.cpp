#include "ReplayManager.h"
#include "Utils.h"
#include "Exceptions.h"
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

ReplayManager::ReplayManager(const std::string& dir)
    : directory(dir), textFilename(""), htmlFilename(""), active(false) {}

void ReplayManager::start(const std::string& playerName,
                          const std::string& trickName,
                          const std::string& modeName) {
    ensureDirectory();
    active = true;
    lines.clear();

    // 生成安全的文件名，包含时间戳和玩家名字
    std::string safePlayer = sanitizeFilePart(playerName);
    std::string safeTime = sanitizeFilePart(Utils::getCurrentTimestamp());
    textFilename = directory + "/replay_" + safeTime + "_" + safePlayer + ".txt";
    htmlFilename = directory + "/replay_" + safeTime + "_" + safePlayer + ".html";

    // 记录回放的基本信息
    lines.push_back("21张牌魔术回放");
    lines.push_back("玩家: " + playerName);
    lines.push_back("魔术类型: " + trickName);
    lines.push_back("模式: " + modeName);
    lines.push_back("时间: " + Utils::getCurrentTimestamp());
    lines.push_back("------------------------------------------------------------");
}

void ReplayManager::recordRound(int round,
                                const std::string& pile1,
                                const std::string& pile2,
                                const std::string& pile3,
                                int chosenPile) {
    if (!active) {
        return;
    }

    lines.push_back("");
    lines.push_back("第 " + std::to_string(round) + " 轮");
    lines.push_back("牌堆 1: " + pile1);
    lines.push_back("牌堆 2: " + pile2);
    lines.push_back("牌堆 3: " + pile3);
    lines.push_back("选择牌堆: " + std::to_string(chosenPile));
}

void ReplayManager::recordReveal(const std::string& card, bool correct, int finalScore) {
    if (!active) {
        return;
    }

    lines.push_back("");
    lines.push_back("揭示牌: " + card);
    lines.push_back(std::string("结果: ") + (correct ? "成功" : "失败"));
    lines.push_back("最终分数: " + std::to_string(finalScore));
    lines.push_back("------------------------------------------------------------");
}

void ReplayManager::save() const {
    if (!active || textFilename.empty()) {
        return;
    }

    // 将回放记录写入文本文件
    std::ofstream file(textFilename);
    if (!file) {
        throw FileIOException(textFilename, "写入回放记录");
    }

    for (const auto& line : lines) {
        file << line << "\n";
    }
}

void ReplayManager::exportHtml() const {
    if (!active || htmlFilename.empty()) {
        return;
    }

    std::ofstream file(htmlFilename);
    if (!file) {
        throw FileIOException(htmlFilename, "写入HTML回放报告");
    }

    file << "<!doctype html>\n<html lang=\"zh-CN\">\n<head>\n";
    file << "<meta charset=\"utf-8\">\n";
    file << "<title>21张牌魔术回放</title>\n";
    file << "<style>";
    file << "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;margin:32px;line-height:1.6;color:#17202a;background:#f7f9fb;}";
    file << "main{max-width:920px;margin:auto;background:#fff;border:1px solid #d9e2ec;border-radius:8px;padding:28px;}";
    file << "h1{font-size:28px;margin:0 0 18px;}pre{white-space:pre-wrap;font-size:15px;background:#f2f5f8;border-radius:6px;padding:18px;}";
    file << "</style>\n</head>\n<body><main>\n";
    file << "<h1>21张牌魔术回放</h1>\n<pre>";
    for (const auto& line : lines) {
        file << escapeHtml(line) << "\n";
    }
    file << "</pre>\n</main></body>\n</html>\n";
}

bool ReplayManager::isActive() const {
    return active;
}

std::string ReplayManager::getTextFilename() const {
    return textFilename;
}

std::string ReplayManager::getHtmlFilename() const {
    return htmlFilename;
}

// 列出某个目录下所有 .txt 回放文件，并按文件名倒序排列后返回
std::vector<std::string> ReplayManager::listReplayFiles(const std::string& dir) {
    // 准备一个数组，用来保存找到的文件路径
    std::vector<std::string> files;
    // 打开目录
    DIR* directoryHandle = opendir(dir.c_str());
    if (!directoryHandle) {
        return files;
    }

    // 循环读取目录里的每一个条目
    dirent* entry = nullptr;
    while ((entry = readdir(directoryHandle)) != nullptr) {
        std::string name = entry->d_name;
        // 只关心以 .txt 结尾的文件，这些是回放记录
        if (name.size() > 4 && name.substr(name.size() - 4) == ".txt") {
            files.push_back(dir + "/" + name);
        }
    }

    // 关闭目录句柄
    closedir(directoryHandle);
    // 按文件名倒序排序，最新的回放文件会排在前面
    std::sort(files.begin(), files.end(), std::greater<std::string>());
    return files;
}

void ReplayManager::displayReplayFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw FileIOException(filename, "读取回放记录");
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << "\n";
    }
}

void ReplayManager::ensureDirectory() const {
    struct stat info;
    // 检查目录是否存在且是一个目录
    if (stat(directory.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)) {
        return;
    }

    // 目录不存在，尝试创建
    if (mkdir(directory.c_str(), 0755) != 0) {
        throw FileIOException(directory, "创建回放目录");
    }
}

// 把字符串清理成适合当文件名一部分的内容
std::string ReplayManager::sanitizeFilePart(const std::string& value) {
    std::ostringstream oss;
    for (char ch : value) {
        unsigned char uch = static_cast<unsigned char>(ch);
        // 文件名只直接保留 ASCII 字母数字，避免中文/花色等 UTF-8 字节在路径里变成乱码。
        if ((uch >= '0' && uch <= '9') ||
            (uch >= 'A' && uch <= 'Z') ||
            (uch >= 'a' && uch <= 'z')) {
            oss << ch;
        } else if (uch == ' ' || uch == '-' || uch == '_' || uch == ':' || uch == '.') {
            oss << '_';
        } else {
            oss << 'x' << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(uch) << std::dec;
        }
    }

    std::string result = oss.str();
    // 如果清理后结果是空字符串，返回一个默认值，避免生成无效文件名
    if (result.empty()) {
        return "player";
    }
    return result;
}
// 把普通字符串转成安全的 HTML 文本，避免内容破坏网页结构
std::string ReplayManager::escapeHtml(const std::string& value) {
    std::string escaped;
    for (char ch : value) {
        switch (ch) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            default: escaped += ch; break;
        }
    }
    return escaped;
}
