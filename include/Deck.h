#ifndef DECK_H
#define DECK_H

#include "Exceptions.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// Deck模板类
template <typename T>
class Deck {
private:
    T* cards;           // 动态数组存储卡牌
    int capacity;       // 容量
    int size;           // 当前大小

    // 调整容量
    void resize(int newCapacity);

public:
    // 构造函数
    Deck();                                    // 默认构造函数
    explicit Deck(int initialCapacity);       // 指定容量构造函数
    Deck(const Deck& other);                  // 拷贝构造函数（深拷贝）
    Deck(Deck&& other) noexcept;              // 移动构造函数

    // 析构函数 (Destructor)
    ~Deck();

    // 赋值运算符
    Deck& operator=(const Deck& other);       // 拷贝赋值
    Deck& operator=(Deck&& other) noexcept;   // 移动赋值

    // 核心操作
    void addCard(const T& card);              // 添加卡牌
    void removeCard(int index);               // 移除指定位置的卡牌
    T getCard(int index) const;               // 获取指定位置的卡牌
    void shuffle();                           // 洗牌
    void clear();                             // 清空牌堆

    // 查询
    int getSize() const { return size; }
    int getCapacity() const { return capacity; }
    bool isEmpty() const { return size == 0; }

    // 运算符重载
    Deck operator+(const Deck& other) const;  // 合并牌堆
    Deck operator-(int index) const;          // 移除指定位置的牌
    Deck operator*(int times) const;          // 复制牌堆N次

    bool operator==(const Deck& other) const; // 相等比较
    bool operator!=(const Deck& other) const; // 不等比较

    T& operator[](int index);                 // 下标访问（可修改）
    const T& operator[](int index) const;     // 下标访问（只读）

    // 流运算符
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Deck<U>& deck);

    template <typename U>
    friend std::istream& operator>>(std::istream& is, Deck<U>& deck);
};

// ==================== 实现部分 (Implementation) ====================

// 默认构造函数
template <typename T>
Deck<T>::Deck() : cards(nullptr), capacity(52), size(0) {
    cards = new T[capacity];
}

// 指定容量构造函数
template <typename T>
Deck<T>::Deck(int initialCapacity) : cards(nullptr), capacity(initialCapacity), size(0) {
    if (initialCapacity <= 0) {
        throw InvalidCardException("牌组容量必须大于0");
    }
    cards = new T[capacity];
}

// 拷贝构造函数（深拷贝）
template <typename T>
Deck<T>::Deck(const Deck& other) : cards(nullptr), capacity(other.capacity), size(other.size) {
    cards = new T[capacity];
    for (int i = 0; i < size; ++i) {
        cards[i] = other.cards[i];
    }
}

// 移动构造函数
template <typename T>
Deck<T>::Deck(Deck&& other) noexcept
    : cards(other.cards), capacity(other.capacity), size(other.size) {
    other.cards = nullptr;
    other.capacity = 0;
    other.size = 0;
}

// 析构函数
template <typename T>
Deck<T>::~Deck() {
    delete[] cards;
}

// 拷贝赋值运算符
template <typename T>
Deck<T>& Deck<T>::operator=(const Deck& other) {
    if (this != &other) {
        // 释放旧内存
        delete[] cards;

        // 分配新内存并拷贝
        capacity = other.capacity;
        size = other.size;
        cards = new T[capacity];
        for (int i = 0; i < size; ++i) {
            cards[i] = other.cards[i];
        }
    }
    return *this;
}

// 移动赋值运算符
template <typename T>
Deck<T>& Deck<T>::operator=(Deck&& other) noexcept {
    if (this != &other) {
        // 释放旧内存
        delete[] cards;

        // 转移所有权
        cards = other.cards;
        capacity = other.capacity;
        size = other.size;

        // 清空源对象
        other.cards = nullptr;
        other.capacity = 0;
        other.size = 0;
    }
    return *this;
}

// 调整容量
template <typename T>
void Deck<T>::resize(int newCapacity) {
    T* newCards = new T[newCapacity];

    // 复制旧数据到新数组
    int copySize = (size < newCapacity) ? size : newCapacity;
    for (int i = 0; i < copySize; ++i) {
        newCards[i] = cards[i];
    }
    delete[] cards;
    cards = newCards;
    capacity = newCapacity;
    if (size > newCapacity) {
        size = newCapacity;
    }
}

// 添加卡牌
template <typename T>
void Deck<T>::addCard(const T& card) {
    if (size >= capacity) {
        // 容量翻倍
        resize(capacity * 2);
    }
    cards[size++] = card;
}

// 移除卡牌
template <typename T>
void Deck<T>::removeCard(int index) {
    if (index < 0 || index >= size) {
        throw OutOfBoundsException(index, size);
    }
    // 将后面的元素前移
    for (int i = index; i < size - 1; ++i) {
        cards[i] = cards[i + 1];
    }
    --size;
}

// 获取卡牌
template <typename T>
T Deck<T>::getCard(int index) const {
    if (index < 0 || index >= size) {
        throw OutOfBoundsException(index, size);
    }
    return cards[index];
}

// 洗牌
template <typename T>
void Deck<T>::shuffle() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(cards, cards + size, rng);
}

// 清空
template <typename T>
void Deck<T>::clear() {
    size = 0;
}

// 合并牌堆
template <typename T>
Deck<T> Deck<T>::operator+(const Deck& other) const {
    Deck<T> result(size + other.size);
    for (int i = 0; i < size; ++i) {
        result.addCard(cards[i]);
    }
    for (int i = 0; i < other.size; ++i) {
        result.addCard(other.cards[i]);
    }
    return result;
}

// 移除指定位置的牌
template <typename T>
Deck<T> Deck<T>::operator-(int index) const {
    Deck<T> result = *this;
    result.removeCard(index);
    return result;
}

// 复制牌堆N次
template <typename T>
Deck<T> Deck<T>::operator*(int times) const {
    if (times < 0) {
        throw InvalidInputException("重复次数不能为负数");
    }
    Deck<T> result(size * times);
    for (int t = 0; t < times; ++t) {
        for (int i = 0; i < size; ++i) {
            result.addCard(cards[i]);
        }
    }
    return result;
}

// 相等比较
template <typename T>
bool Deck<T>::operator==(const Deck& other) const {
    if (size != other.size) return false;
    for (int i = 0; i < size; ++i) {
        if (cards[i] != other.cards[i]) return false;
    }
    return true;
}

// 不等比较
template <typename T>
bool Deck<T>::operator!=(const Deck& other) const {
    return !(*this == other);
}

// 下标访问（可修改）
template <typename T>
T& Deck<T>::operator[](int index) {
    if (index < 0 || index >= size) {
        throw OutOfBoundsException(index, size);
    }
    return cards[index];
}

// 下标访问（只读）
template <typename T>
const T& Deck<T>::operator[](int index) const {
    if (index < 0 || index >= size) {
        throw OutOfBoundsException(index, size);
    }
    return cards[index];
}

// 输出流运算符
template <typename U>
std::ostream& operator<<(std::ostream& os, const Deck<U>& deck) {
    os << "牌组 [" << deck.size << "/" << deck.capacity << "]: ";
    for (int i = 0; i < deck.size; ++i) {
        os << deck.cards[i];
        if (i < deck.size - 1) os << " ";
    }
    return os;
}

// 输入流运算符
template <typename U>
std::istream& operator>>(std::istream& is, Deck<U>& deck) {
    int count;
    is >> count;
    if (is.fail()) {
        throw InvalidInputException("读取牌组大小失败");
    }
    deck.clear();
    for (int i = 0; i < count; ++i) {
        U card;
        is >> card;
        deck.addCard(card);
    }
    return is;
}

#endif // DECK_H
