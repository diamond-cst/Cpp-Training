#ifndef PILECHOICE_H
#define PILECHOICE_H

#include <iostream>

// 观众叠数输入 (Audience pile choice input)
// 支持 1/2/3 选择牌堆、P/p 暂停、S/s 保存。
class PileChoice {
private:
    int value;  // 选择的牌堆编号，1/2/3 对应三堆，0 表示无效输入。
    bool pauseRequested; // 是否请求暂停，输入P/p时为true。
    bool saveRequested; // 是否请求保存，输入S/s时为true。

public:
    PileChoice();
    explicit PileChoice(int pileValue);

    int getValue() const;
    bool isPauseRequested() const;
    bool isSaveRequested() const;

    // 从输入流读取选择，支持 1/2/3、P/p、S/s，并设置相应的状态
    friend std::istream& operator>>(std::istream& is, PileChoice& choice);
};

#endif // PILECHOICE_H
