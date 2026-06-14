#ifndef PILECHOICE_H
#define PILECHOICE_H

#include <iostream>

// 观众叠数输入 (Audience pile choice input)
// 支持 1/2/3 和 P/p 暂停，满足 >> 输入叠数并做合法性检查的要求。
class PileChoice {
private:
    int value;
    bool pauseRequested;

public:
    PileChoice();

    int getValue() const;
    bool isPauseRequested() const;

    friend std::istream& operator>>(std::istream& is, PileChoice& choice);
};

#endif // PILECHOICE_H
