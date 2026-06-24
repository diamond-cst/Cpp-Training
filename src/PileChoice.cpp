#include "PileChoice.h"
#include "Exceptions.h"
#include <string>

PileChoice::PileChoice() : value(0), pauseRequested(false), saveRequested(false) {}

PileChoice::PileChoice(int pileValue)
    : value(pileValue), pauseRequested(false), saveRequested(false) {
    if (pileValue < 1 || pileValue > 3) {
        throw InvalidInputException("牌堆选择只能是 1、2 或 3");
    }
}

int PileChoice::getValue() const {
    return value;
}

bool PileChoice::isPauseRequested() const {
    return pauseRequested;
}

bool PileChoice::isSaveRequested() const {
    return saveRequested;
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

    // 暂停请求
    if (token == "P" || token == "p") {
        choice.value = 0;
        choice.pauseRequested = true;
        choice.saveRequested = false;
        return is;
    }

    // 保存请求
    if (token == "S" || token == "s") {
        choice.value = 0;
        choice.pauseRequested = false;
        choice.saveRequested = true;
        return is;
    }

    // 牌堆选择
    if (token == "1" || token == "2" || token == "3") {
        choice.value = token[0] - '0';
        choice.pauseRequested = false;
        choice.saveRequested = false;
        return is;
    }

    throw InvalidInputException("牌堆选择只能输入 1、2、3、P 或 S");
}
