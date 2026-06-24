#ifndef CARD_H
#define CARD_H

#include <string>
#include <iostream>

// 卡牌类
class Card {
public:
    // 花色枚举
    enum class Suit {
        HEARTS,    // ♥ 红桃
        DIAMONDS,  // ♦ 方块
        CLUBS,     // ♣ 梅花
        SPADES,    // ♠ 黑桃
        NONE       // 无花色（用于数字牌）
    };

    // 点数枚举
    enum class Rank {
        ACE = 1,   // A
        TWO = 2,
        THREE = 3,
        FOUR = 4,
        FIVE = 5,
        SIX = 6,
        SEVEN = 7,
        EIGHT = 8,
        NINE = 9,
        TEN = 10,
        JACK = 11,  // J
        QUEEN = 12, // Q
        KING = 13,  // K
        NUMERIC = 0 // 数字牌标记
    };

private:
    Suit suit;          // 花色
    Rank rank;          // 点数
    int numericValue;   // 数字值（用于1-21或数字表示）

public:
    // 构造函数：数字牌
    explicit Card(int value); // 避免隐式转换

    // 构造函数：标准扑克牌
    Card(Suit s, Rank r);

    // 默认构造函数
    Card();

    // 拷贝构造函数
    Card(const Card& other);

    // 赋值运算符
    Card& operator=(const Card& other);

    // 析构函数
    ~Card() = default;

    // 获取器
    int getValue() const;
    Suit getSuit() const;
    Rank getRank() const;
    bool isNumeric() const;

    // 转换为字符串
    std::string toString(bool useColor = false) const;

    // 比较运算符
    bool operator==(const Card& other) const;
    bool operator!=(const Card& other) const;
    bool operator<(const Card& other) const;
    bool operator>(const Card& other) const;
    bool operator<=(const Card& other) const;
    bool operator>=(const Card& other) const;

    // 流运算符
    friend std::ostream& operator<<(std::ostream& os, const Card& card);
    friend std::istream& operator>>(std::istream& is, Card& card);

private:
    // 辅助函数：获取花色符号
    std::string getSuitSymbol() const;

    // 辅助函数：获取点数字符串
    std::string getRankString() const;

    // 辅助函数：获取花色颜色代码
    std::string getColorCode() const;
};

#endif // CARD_H
