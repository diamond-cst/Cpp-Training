#ifndef PILECHOICE_H
#define PILECHOICE_H

#include <iostream>

// 观众叠数输入 (Audience pile choice input)
// 支持 1/2/3 选择牌堆、P/p 暂停、S/s 保存。
class PileChoice {
private:
    int value;
    bool pauseRequested;
    bool saveRequested;

public:
    PileChoice();
    explicit PileChoice(int pileValue);

    int getValue() const;
    bool isPauseRequested() const;
    bool isSaveRequested() const;

    friend std::istream& operator>>(std::istream& is, PileChoice& choice);
};

#endif // PILECHOICE_H
