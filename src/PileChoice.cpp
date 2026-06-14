#include "PileChoice.h"
#include "Exceptions.h"
#include <string>

PileChoice::PileChoice() : value(0), pauseRequested(false) {}

int PileChoice::getValue() const {
    return value;
}

bool PileChoice::isPauseRequested() const {
    return pauseRequested;
}

std::istream& operator>>(std::istream& is, PileChoice& choice) {
    std::string token;
    is >> token;
    if (is.fail()) {
        if (is.eof()) {
            throw InvalidInputException("读取牌堆选择时输入已结束");
        }
        throw InvalidInputException("读取牌堆选择失败");
    }

    if (token == "P" || token == "p") {
        choice.value = 0;
        choice.pauseRequested = true;
        return is;
    }

    if (token == "1" || token == "2" || token == "3") {
        choice.value = token[0] - '0';
        choice.pauseRequested = false;
        return is;
    }

    throw InvalidInputException("牌堆选择只能输入 1、2、3 或 P");
}
