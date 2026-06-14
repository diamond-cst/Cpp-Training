#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

// 基础魔术异常类 (Base magic trick exception class)
class MagicTrickException : public std::exception {
protected:
    std::string message;

public:
    explicit MagicTrickException(const std::string& msg) : message(msg) {}
    virtual ~MagicTrickException() noexcept = default;

    const char* what() const noexcept override {
        return message.c_str();
    }
};

// 无效卡牌异常 (Invalid card exception)
class InvalidCardException : public MagicTrickException {
public:
    explicit InvalidCardException(const std::string& msg)
        : MagicTrickException("无效卡牌: " + msg) {}
};

// 越界异常 (Out of bounds exception)
class OutOfBoundsException : public MagicTrickException {
public:
    explicit OutOfBoundsException(int index, int size)
        : MagicTrickException("索引 " + std::to_string(index) +
                            " 超出范围 [0, " + std::to_string(size - 1) + "]") {}
};

// 文件IO异常 (File I/O exception)
class FileIOException : public MagicTrickException {
public:
    explicit FileIOException(const std::string& filename, const std::string& operation)
        : MagicTrickException("文件读写错误: " + operation + "，文件: " + filename) {}
};

// 无效游戏状态异常 (Invalid game state exception)
class InvalidGameStateException : public MagicTrickException {
public:
    explicit InvalidGameStateException(const std::string& msg)
        : MagicTrickException("无效游戏状态: " + msg) {}
};

// 无效输入异常 (Invalid input exception)
class InvalidInputException : public MagicTrickException {
public:
    explicit InvalidInputException(const std::string& msg)
        : MagicTrickException("无效输入: " + msg) {}
};

#endif // EXCEPTIONS_H
