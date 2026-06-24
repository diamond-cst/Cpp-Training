#include "Card.h"
#include "Exceptions.h"
#include <sstream>

// 构造函数：数字牌
Card::Card(int value) : suit(Suit::NONE), rank(Rank::NUMERIC), numericValue(value) {
    if (value < 1) {
        throw InvalidCardException("数字牌的数值必须至少为1");
    }
}

// 构造函数：标准扑克牌
Card::Card(Suit s, Rank r) : suit(s), rank(r), numericValue(0) {
    if (s == Suit::NONE || r == Rank::NUMERIC) {
        throw InvalidCardException("标准扑克牌的花色或点数无效");
    }
    // 计算数字值用于比较
    numericValue = static_cast<int>(r);
}

// 默认构造函数
Card::Card() : suit(Suit::NONE), rank(Rank::NUMERIC), numericValue(0) {}

// 拷贝构造函数
Card::Card(const Card& other)
    : suit(other.suit), rank(other.rank), numericValue(other.numericValue) {}

// 赋值运算符
Card& Card::operator=(const Card& other) {
    if (this != &other) {
        suit = other.suit;
        rank = other.rank;
        numericValue = other.numericValue;
    }
    return *this;
}

// 获取数字值
int Card::getValue() const {
    return numericValue;
}

// 获取花色
Card::Suit Card::getSuit() const {
    return suit;
}

// 获取点数
Card::Rank Card::getRank() const {
    return rank;
}

// 是否为数字牌
bool Card::isNumeric() const {
    return rank == Rank::NUMERIC;
}

// 获取花色符号
std::string Card::getSuitSymbol() const {
    switch (suit) {
        case Suit::HEARTS:   return "♥";
        case Suit::DIAMONDS: return "♦";
        case Suit::CLUBS:    return "♣";
        case Suit::SPADES:   return "♠";
        case Suit::NONE:     return "";
        default:             return "?";
    }
}

// 获取点数字符串
std::string Card::getRankString() const {
    if (rank == Rank::NUMERIC) {
        return std::to_string(numericValue);
    }

    switch (rank) {
        case Rank::ACE:   return "A";
        case Rank::JACK:  return "J";
        case Rank::QUEEN: return "Q";
        case Rank::KING:  return "K";
        default:          return std::to_string(static_cast<int>(rank));
    }
}

// 获取颜色代码
std::string Card::getColorCode() const {
    // 红色花色：红桃和方块
    if (suit == Suit::HEARTS || suit == Suit::DIAMONDS) {
        return "\033[31m"; // 红色
    }
    // 黑色花色：梅花和黑桃
    else if (suit == Suit::CLUBS || suit == Suit::SPADES) {
        return "\033[30m"; // 黑色
    }
    return "\033[0m"; // 默认颜色
}

// 转换为字符串
std::string Card::toString(bool useColor) const {
    std::ostringstream oss;

    if (useColor) {
        oss << getColorCode();
    }

    if (isNumeric()) {
        oss << "[" << numericValue << "]";
    } else {
        oss << "[" << getRankString() << getSuitSymbol() << "]";
    }

    if (useColor) {
        oss << "\033[0m"; // 重置颜色
    }

    return oss.str();
}

// 比较运算符：相等
bool Card::operator==(const Card& other) const {
    if (isNumeric() && other.isNumeric()) {
        return numericValue == other.numericValue;
    }
    return suit == other.suit && rank == other.rank;
}

// 比较运算符：不等
bool Card::operator!=(const Card& other) const {
    return !(*this == other);
}

// 比较运算符：小于
bool Card::operator<(const Card& other) const {
    return numericValue < other.numericValue;
}

// 比较运算符：大于
bool Card::operator>(const Card& other) const {
    return numericValue > other.numericValue;
}

// 比较运算符：小于等于
bool Card::operator<=(const Card& other) const {
    return numericValue <= other.numericValue;
}

// 比较运算符：大于等于
bool Card::operator>=(const Card& other) const {
    return numericValue >= other.numericValue;
}

// 输出流运算符
std::ostream& operator<<(std::ostream& os, const Card& card) {
    os << card.toString(false);
    return os;
}

// 输入流运算符
std::istream& operator>>(std::istream& is, Card& card) {
    int value;
    is >> value;
    if (is.fail()) {
        throw InvalidInputException("读取牌面失败");
    }
    card = Card(value);
    return is;
}
