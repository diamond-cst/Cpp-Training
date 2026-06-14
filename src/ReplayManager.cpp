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

    std::string safePlayer = sanitizeFilePart(playerName);
    std::string safeTime = sanitizeFilePart(Utils::getCurrentTimestamp());
    textFilename = directory + "/replay_" + safeTime + "_" + safePlayer + ".txt";
    htmlFilename = directory + "/replay_" + safeTime + "_" + safePlayer + ".html";

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

std::vector<std::string> ReplayManager::listReplayFiles(const std::string& dir) {
    std::vector<std::string> files;
    DIR* directoryHandle = opendir(dir.c_str());
    if (!directoryHandle) {
        return files;
    }

    dirent* entry = nullptr;
    while ((entry = readdir(directoryHandle)) != nullptr) {
        std::string name = entry->d_name;
        if (name.size() > 4 && name.substr(name.size() - 4) == ".txt") {
            files.push_back(dir + "/" + name);
        }
    }

    closedir(directoryHandle);
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
    if (stat(directory.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)) {
        return;
    }

    if (mkdir(directory.c_str(), 0755) != 0) {
        throw FileIOException(directory, "创建回放目录");
    }
}

std::string ReplayManager::sanitizeFilePart(const std::string& value) {
    std::ostringstream oss;
    for (char ch : value) {
        unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch)) {
            oss << ch;
        } else {
            oss << '_';
        }
    }

    std::string result = oss.str();
    if (result.empty()) {
        return "player";
    }
    return result;
}

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
