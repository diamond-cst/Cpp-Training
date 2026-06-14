#include "Card.h"
#include "Exceptions.h"
#include <sstream>

// 构造函数：数字牌 (Constructor for numeric cards)
Card::Card(int value) : suit(Suit::NONE), rank(Rank::NUMERIC), numericValue(value) {
    if (value < 1) {
        throw InvalidCardException("Numeric value must be at least 1");
    }
}

// 构造函数：标准扑克牌 (Constructor for standard playing cards)
Card::Card(Suit s, Rank r) : suit(s), rank(r), numericValue(0) {
    if (s == Suit::NONE || r == Rank::NUMERIC) {
        throw InvalidCardException("Invalid suit or rank for standard card");
    }
    // 计算数字值用于比较 (Calculate numeric value for comparison)
    numericValue = static_cast<int>(r);
}

// 默认构造函数 (Default constructor)
Card::Card() : suit(Suit::NONE), rank(Rank::NUMERIC), numericValue(0) {}

// 拷贝构造函数 (Copy constructor)
Card::Card(const Card& other)
    : suit(other.suit), rank(other.rank), numericValue(other.numericValue) {}

// 赋值运算符 (Assignment operator)
Card& Card::operator=(const Card& other) {
    if (this != &other) {
        suit = other.suit;
        rank = other.rank;
        numericValue = other.numericValue;
    }
    return *this;
}

// 获取数字值 (Get numeric value)
int Card::getValue() const {
    return numericValue;
}

// 获取花色 (Get suit)
Card::Suit Card::getSuit() const {
    return suit;
}

// 获取点数 (Get rank)
Card::Rank Card::getRank() const {
    return rank;
}

// 是否为数字牌 (Check if numeric card)
bool Card::isNumeric() const {
    return rank == Rank::NUMERIC;
}

// 获取花色符号 (Get suit symbol)
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

// 获取点数字符串 (Get rank string)
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

// 获取颜色代码 (Get ANSI color code)
std::string Card::getColorCode() const {
    // 红色花色：红桃和方块 (Red suits: hearts and diamonds)
    if (suit == Suit::HEARTS || suit == Suit::DIAMONDS) {
        return "\033[31m"; // 红色 (Red)
    }
    // 黑色花色：梅花和黑桃 (Black suits: clubs and spades)
    else if (suit == Suit::CLUBS || suit == Suit::SPADES) {
        return "\033[30m"; // 黑色 (Black)
    }
    return "\033[0m"; // 默认颜色 (Default color)
}

// 转换为字符串 (Convert to string)
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
        oss << "\033[0m"; // 重置颜色 (Reset color)
    }

    return oss.str();
}

// 比较运算符：相等 (Equality operator)
bool Card::operator==(const Card& other) const {
    if (isNumeric() && other.isNumeric()) {
        return numericValue == other.numericValue;
    }
    return suit == other.suit && rank == other.rank;
}

// 比较运算符：不等 (Inequality operator)
bool Card::operator!=(const Card& other) const {
    return !(*this == other);
}

// 比较运算符：小于 (Less than operator)
bool Card::operator<(const Card& other) const {
    return numericValue < other.numericValue;
}

// 比较运算符：大于 (Greater than operator)
bool Card::operator>(const Card& other) const {
    return numericValue > other.numericValue;
}

// 比较运算符：小于等于 (Less than or equal operator)
bool Card::operator<=(const Card& other) const {
    return numericValue <= other.numericValue;
}

// 比较运算符：大于等于 (Greater than or equal operator)
bool Card::operator>=(const Card& other) const {
    return numericValue >= other.numericValue;
}

// 输出流运算符 (Output stream operator)
std::ostream& operator<<(std::ostream& os, const Card& card) {
    os << card.toString(false);
    return os;
}

// 输入流运算符 (Input stream operator)
std::istream& operator>>(std::istream& is, Card& card) {
    int value;
    is >> value;
    if (is.fail()) {
        throw InvalidInputException("Failed to read card value");
    }
    card = Card(value);
    return is;
}
