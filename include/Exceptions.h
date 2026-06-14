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
        : MagicTrickException("Invalid Card: " + msg) {}
};

// 越界异常 (Out of bounds exception)
class OutOfBoundsException : public MagicTrickException {
public:
    explicit OutOfBoundsException(int index, int size)
        : MagicTrickException("Index " + std::to_string(index) +
                            " out of bounds [0, " + std::to_string(size - 1) + "]") {}
};

// 文件IO异常 (File I/O exception)
class FileIOException : public MagicTrickException {
public:
    explicit FileIOException(const std::string& filename, const std::string& operation)
        : MagicTrickException("File I/O error: " + operation + " on " + filename) {}
};

// 无效游戏状态异常 (Invalid game state exception)
class InvalidGameStateException : public MagicTrickException {
public:
    explicit InvalidGameStateException(const std::string& msg)
        : MagicTrickException("Invalid Game State: " + msg) {}
};

// 无效输入异常 (Invalid input exception)
class InvalidInputException : public MagicTrickException {
public:
    explicit InvalidInputException(const std::string& msg)
        : MagicTrickException("Invalid Input: " + msg) {}
};

#endif // EXCEPTIONS_H
