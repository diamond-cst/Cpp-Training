#include "NetworkGame.h"
#include "Exceptions.h"
#include "TwentyOneCardTrick.h"
#include "Utils.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace {
const int BACKLOG = 1; // 最大连接队列长度

// 返回带有错误描述的字符串，便于抛出异常
std::string socketError(const std::string& action) {
    return action + ": " + std::strerror(errno);
}

// 显示观众端初始状态，包括牌堆和提示信息
void displayAudienceInitialState(const std::vector<std::string>& deck) {
    Utils::clearScreen();
    Utils::printTitle("21张牌魔术");
    Utils::printColored("玩家: ", Utils::COLOR_CYAN);
    std::cout << "远程观众\n";
    Utils::printColored("模式: ", Utils::COLOR_CYAN);
    std::cout << "观众互动\n\n";

    Utils::printColored("这是21张牌:\n", Utils::COLOR_GREEN);
    for (size_t i = 0; i < deck.size(); ++i) {
        std::cout << deck[i] << " ";
        if ((i + 1) % 7 == 0) {
            std::cout << "\n";
        }
    }

    Utils::printColored("\n请在心中记住其中一张牌，但不要告诉魔术师！\n",
                        Utils::COLOR_YELLOW);
}

// 显示观众端每轮的牌堆状态和提示信息
void displayAudiencePiles(int round, const std::vector<std::vector<std::string>>& piles) {
    const int dealDelayMs = 35;

    Utils::clearScreen();
    Utils::printSeparator('=', 60);
    Utils::printStyled("  第 " + std::to_string(round) + " 轮\n",
                       Utils::COLOR_YELLOW, Utils::BOLD);
    Utils::printSeparator('=', 60);

    Utils::printColored("\n正在发牌，请观察每张牌的位置...\n\n",
                        Utils::COLOR_YELLOW);
    for (size_t i = 0; i < piles.size(); ++i) {
        Utils::printColored("牌堆 " + std::to_string(i + 1) + ":  ", Utils::COLOR_CYAN);
        for (const auto& card : piles[i]) {
            std::cout << card << " " << std::flush;
            Utils::playDealSound();
            Utils::sleep(dealDelayMs);
        }
        std::cout << "\n";
    }
}
}

class NetworkMagicianTrick : public TwentyOneCardTrick {
private:
    int clientFd; // 与观众端通信的socket文件描述符

    // 将当前三堆牌转换为字符串列表，便于网络传输
    std::vector<std::vector<std::string>> currentPilesAsText() const {
        // 创建一个二维数组，里面有 3 个 vector<string>，分别对应三堆牌
        std::vector<std::vector<std::string>> piles(3);
        // 创建一个数组，保存三个牌堆的地址。
        const Deck<Card>* sourcePiles[] = {&pile1, &pile2, &pile3};
        for (int pileIndex = 0; pileIndex < 3; ++pileIndex) {
            for (int i = 0; i < sourcePiles[pileIndex]->getSize(); ++i) {
                // 将每张牌转换为字符串，并添加到对应的牌堆列表中
                piles[pileIndex].push_back(sourcePiles[pileIndex]->getCard(i).toString(useColors));
            }
        }
        return piles;
    }

    // 将当前工作牌堆转换为单行字符串，便于网络传输
    std::string workingDeckToString() const {
        // 将工作牌堆中的每张牌转换为字符串，并收集到一个列表中
        std::vector<std::string> cards;
        // 这里直接使用 workingDeck 的 getCard 方法获取每张牌，并调用 toString 转换为字符串
        for (int i = 0; i < workingDeck.getSize(); ++i) {
            cards.push_back(workingDeck.getCard(i).toString(useColors));
        }
        // 使用 NetworkGame::joinCards 将牌列表拼接成一个单行字符串
        return NetworkGame::joinCards(cards);
    }

protected:
    // 覆盖父类的虚函数，根据网络交互获取观众选择的牌堆，并确认揭晓结果
    PileChoice requestAudienceChoice(int roundNumber) override {
        // 发送当前轮的牌堆信息给观众端，并等待观众选择
        NetworkGame::sendPiles(clientFd, roundNumber, currentPilesAsText());
        Utils::printColored("\n等待观众选择牌堆...\n",
                            Utils::COLOR_CYAN);

        // 接收观众端发送的选择消息，格式为 "CHOICE|<pileNumber>"
        std::string line = NetworkGame::receiveLine(clientFd);
        if (line.find("CHOICE|") != 0) {
            throw InvalidGameStateException("收到异常网络消息: " + line);
        }

        // 解析观众选择的牌堆编号，并进行范围检查
        int chosenPile = std::stoi(line.substr(7));
        if (chosenPile < 1 || chosenPile > 3) {
            throw InvalidInputException("观众选择的牌堆超出范围");
        }

        Utils::printColored("观众选择了第 " + std::to_string(chosenPile) + " 堆。\n",
                            Utils::COLOR_YELLOW);
        // 发送确认消息给观众端，告知已收到选择
        NetworkGame::sendLine(clientFd, "ACK|ROUND|" + std::to_string(roundNumber));
        // 返回观众选择的牌堆编号，供父类继续处理
        return PileChoice(chosenPile);
    }

    // 覆盖父类的虚函数，根据网络交互确认观众揭晓结果
    bool confirmAudienceReveal(const Card& revealedCard) override {
        // 发送揭晓结果给观众端，格式为 "REVEAL|<card>"
        NetworkGame::sendLine(clientFd, "REVEAL|" + revealedCard.toString(useColors));
        // 接收观众端发送的确认消息，格式为 "CONFIRM|<result>"
        std::string line = NetworkGame::receiveLine(clientFd);
        if (line.find("CONFIRM|") != 0) {
            throw InvalidGameStateException("未收到观众端的揭晓确认");
        }

        // 解析观众确认结果，'Y' 表示正确，'N' 表示错误
        char result = line.size() > 8 ? line[8] : 'N';
        // 返回观众确认结果，供父类继续处理
        return result == 'Y' || result == 'y';
    }

public:
    // 构造函数，初始化魔术师端，传入与观众端通信的socket文件描述符
    explicit NetworkMagicianTrick(int socketFd)
        : TwentyOneCardTrick(true), clientFd(socketFd) {}

    // 覆盖父类的虚函数，初始化魔术师端，发送初始牌组给观众端
    void initialize() override {
        // 调用父类的初始化方法，发送初始牌组给观众端
        TwentyOneCardTrick::initialize();
        // 发送初始牌组给观众端，格式为 "DECK|<cards>"
        NetworkGame::sendLine(clientFd, "DECK|" + workingDeckToString());
    }
};

// 构造函数，初始化SocketHandle，设置fd为-1
NetworkGame::SocketHandle::SocketHandle() : fd(-1) {}
// 构造函数，初始化SocketHandle，设置fd为传入的socket文件描述符
NetworkGame::SocketHandle::SocketHandle(int socketFd) : fd(socketFd) {}

// 析构函数，重置SocketHandle，关闭socket连接
NetworkGame::SocketHandle::~SocketHandle() {
    // 调用reset方法，关闭socket连接
    reset();
}

// 移动构造函数，转移socket所有权
NetworkGame::SocketHandle::SocketHandle(SocketHandle&& other) noexcept : fd(other.fd) {
    other.fd = -1;
}

// 移动赋值运算符，转移socket所有权
NetworkGame::SocketHandle& NetworkGame::SocketHandle::operator=(SocketHandle&& other) noexcept {
    if (this != &other) {
        // 调用reset方法，关闭旧的socket连接
        reset();
        // 转移socket所有权
        fd = other.fd;
        // 将旧的socket文件描述符设置为-1，避免重复关闭同一个socket
        other.fd = -1;
    }
    return *this;
}

// 检查socket是否有效
bool NetworkGame::SocketHandle::isValid() const {
    return fd >= 0;
}

// 释放socket所有权，返回fd并将fd置为-1，避免析构时关闭连接
int NetworkGame::SocketHandle::release() {
    // 保存当前socket文件描述符
    int value = fd;
    // 将socket文件描述符设置为-1，避免析构时关闭连接
    fd = -1;
    // 返回保存的socket文件描述符
    return value;
}

// 重置socket，关闭旧的fd并设置新的fd，默认关闭当前socket
void NetworkGame::SocketHandle::reset(int socketFd) {
    // 如果当前socket文件描述符有效，则关闭socket连接
    if (fd >= 0) {
        // 关闭socket连接
        close(fd);
    }
    // 设置新的socket文件描述符
    fd = socketFd;
}

// 运行魔术师服务器，监听指定端口，等待观众连接
void NetworkGame::runMagicianServer(int port) {
    // 检查端口号是否在有效范围内
    if (port < 1024 || port > 65535) {
        throw InvalidInputException("端口号必须在1024到65535之间");
    }

    // 创建服务器套接字
    // ipv4地址族，流式套接字，协议为0（默认TCP）
    SocketHandle server(socket(AF_INET, SOCK_STREAM, 0));
    if (!server.isValid()) {
        throw MagicTrickException(socketError("创建服务器套接字失败"));
    }

    // 开启端口复用，避免服务端刚关闭又重启时，因为端口暂时占用导致绑定失败
    int opt = 1;
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 定义一个ipv4地址，用于绑定服务器端口
    // sockaddr_in 专门用来保存 IPv4 socket 地址信息，包括地址类型、地址、端口
    sockaddr_in address;
    // 初始化地址为0
    std::memset(&address, 0, sizeof(address));
    // 设置地址类型为ipv4
    address.sin_family = AF_INET;
    // 设置地址为任意地址，即0.0.0.0，表示本机所有网卡
    address.sin_addr.s_addr = INADDR_ANY;
    // 设置端口号，转换为网络字节序
    address.sin_port = htons(static_cast<uint16_t>(port));

    // 绑定服务器端口，将套接字与地址绑定
    if (bind(server.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("绑定服务器端口失败"));
    }

    // 监听服务器端口，等待观众连接
    if (listen(server.fd, BACKLOG) < 0) {
        throw MagicTrickException(socketError("监听服务器端口失败"));
    }

    Utils::clearScreen();
    Utils::printTitle("21张牌魔术");
    Utils::printColored("\n等待观众进入房间...\n", Utils::COLOR_GREEN);
    Utils::printColored("房间端口: " + std::to_string(port) + "\n",
                        Utils::COLOR_CYAN);

    // 设置客户端地址
    sockaddr_in clientAddress;
    // 设置客户端地址长度
    socklen_t clientLength = sizeof(clientAddress);
    // 接收观众连接，返回客户端套接字
    SocketHandle client(accept(server.fd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength));
    // 如果客户端套接字无效，则抛出异常
    if (!client.isValid()) {
        throw MagicTrickException(socketError("接收观众连接失败"));
    }

    // 创建一个字符数组，用于保存观众IP地址
    char clientIp[INET_ADDRSTRLEN] = {0};
    // 把客户端 IP 地址从二进制格式转换成可读字符串
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));
    Utils::printColored("观众已加入，游戏即将开始。\n", Utils::COLOR_GREEN);
    Utils::printColored("连接来源: ", Utils::COLOR_CYAN);
    std::cout << clientIp << "\n";

    // 发送欢迎消息给观众端，格式为 "WELCOME|21张牌网络对战"
    NetworkGame::sendLine(client.fd, "WELCOME|21张牌网络对战");

    // 创建一个NetworkMagicianTrick对象，用于处理魔术师端逻辑
    NetworkMagicianTrick trick(client.fd);
    trick.setPlayerName("网络魔术师");
    trick.setUseAnimation(true);
    trick.setSoundEnabled(true);
    trick.initialize();

    // 执行三轮魔术
    while (!trick.isComplete()) {
        // 执行一轮魔术
        trick.performRound();
    }

    // 揭晓结果
    trick.reveal();
    // 打印本局双人魔术结束
    Utils::printColored("本局双人魔术结束。\n", Utils::COLOR_GREEN);
}

// 运行观众客户端，连接指定主机和端口
void NetworkGame::runAudienceClient(const std::string& host, int port) {
    // 检查主机地址是否为空
    if (host.empty()) {
        // 如果主机地址为空，则抛出异常
        throw InvalidInputException("主机地址不能为空");
    }
    // 检查端口号是否在有效范围内
    if (port < 1024 || port > 65535) {
        // 如果端口号不在有效范围内，则抛出异常
        throw InvalidInputException("端口号必须在1024到65535之间");
    }

    // 创建一个客户端套接字
    SocketHandle client(socket(AF_INET, SOCK_STREAM, 0));
    // 如果客户端套接字无效，则抛出异常
    if (!client.isValid()) {
        throw MagicTrickException(socketError("创建客户端套接字失败"));
    }

    // 定义一个ipv4地址，用于连接魔术师端
    sockaddr_in address;
    // 初始化地址为0
    std::memset(&address, 0, sizeof(address));
    // 设置地址类型为ipv4
    address.sin_family = AF_INET;
    // 设置端口号，转换为网络字节序
    address.sin_port = htons(static_cast<uint16_t>(port));

    // 把主机地址从字符串格式转换为二进制格式
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
        throw InvalidInputException("主机地址必须是IPv4格式，例如127.0.0.1");
    }

    Utils::printColored("\n正在进入魔术师房间...\n", Utils::COLOR_CYAN);
    // 连接魔术师端，返回连接结果
    if (connect(client.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("连接魔术师端失败"));
    }

    // 接收欢迎消息，格式为 "WELCOME|21张牌网络对战"
    std::string welcome = receiveLine(client.fd);
    if (welcome.find("WELCOME|") == 0) {
        Utils::printColored("已进入房间，等待魔术师发牌...\n", Utils::COLOR_GREEN);
    }

    // 接收初始牌组，格式为 "DECK|<cards>"
    std::string deckLine = receiveLine(client.fd);
    if (deckLine.find("DECK|") != 0) {
        throw InvalidGameStateException("未收到魔术师端发送的初始牌组");
    }

    // 显示观众端初始状态，包括牌堆和提示信息
    displayAudienceInitialState(splitCards(deckLine.substr(5)));
    // 等待用户按任意键继续
    Utils::pressAnyKey();

    // 执行三轮魔术
    for (int expectedRound = 1; expectedRound <= 3; ++expectedRound) {
        // 接收当前轮的牌堆信息
        int round = 0;
        // 接收当前轮的牌堆信息，返回牌堆列表
        auto piles = receivePiles(client.fd, round);
        if (round != expectedRound) {
            throw InvalidGameStateException("魔术师端发送的轮次不正确");
        }

        // 显示观众端每轮的牌堆状态和提示信息
        displayAudiencePiles(round, piles);

        // 等待用户输入选择的牌堆编号
        int choice = Utils::getIntInput("\n你记住的牌在哪一堆？请选择: ", 1, 3);
        // 发送选择的牌堆编号给魔术师端，格式为 "CHOICE|<pileNumber>"
        sendLine(client.fd, "CHOICE|" + std::to_string(choice));

        // 接收魔术师端发送的本轮确认消息，格式为 "ACK|ROUND|<roundNumber>"
        std::string ack = receiveLine(client.fd);
        if (ack.find("ACK|ROUND|") != 0) {
            throw InvalidGameStateException("未收到魔术师端的本轮确认");
        }
    }

    // 接收揭晓结果，格式为 "REVEAL|<card>"
    std::string revealLine = receiveLine(client.fd);
    // 如果揭晓结果格式不正确，则抛出异常
    if (revealLine.find("REVEAL|") != 0) {
        throw InvalidGameStateException("未收到魔术师端的揭晓结果");
    }

    Utils::clearScreen();
    Utils::printTitle("揭晓时刻！");
    Utils::printColored("魔术师认为你记住的牌是第11张牌...\n",
                        Utils::COLOR_YELLOW);
    std::cout << "\n";
    Utils::printStyled("它是: " + revealLine.substr(7) + "\n",
                       Utils::COLOR_GREEN, Utils::BOLD);
    bool correct = Utils::confirm("魔术师猜对了吗？");
    // 发送确认消息给魔术师端，格式为 "CONFIRM|<result>"
    sendLine(client.fd, std::string("CONFIRM|") + (correct ? "Y" : "N"));
}

// 将牌堆列表拼接为单行字符串，便于网络传输
std::string NetworkGame::joinCards(const std::vector<std::string>& cards) {
    // 创建一个字符串流
    std::ostringstream oss;
    for (size_t i = 0; i < cards.size(); ++i) {
        // 如果当前牌不是第一张，则添加一个空格
        if (i > 0) {
            // 添加一个空格
            oss << " ";
        }
        // 添加当前牌
        oss << cards[i];
    }
    // 返回拼接后的字符串
    return oss.str();
}

// 将单行字符串拆分为牌堆列表，便于网络传输
std::vector<std::string> NetworkGame::splitCards(const std::string& cards) {
    std::vector<std::string> result;
    // 创建一个字符串流
    std::istringstream iss(cards);
    // 创建一个字符串，用于保存当前牌
    std::string card;
    // 当字符串流中还有牌时，继续读取
    while (iss >> card) {
        // 添加当前牌
        result.push_back(card);
    }
    // 返回拆分后的牌堆列表
    return result;
}

// 发送单行消息
void NetworkGame::sendLine(int socketFd, const std::string& line) {
    // 添加换行符
    std::string payload = line + "\n";
    // 转换为字符数组
    const char* data = payload.c_str();
    // 剩余字节数
    size_t remaining = payload.size();

    // 当剩余字节数大于0时，继续发送
    while (remaining > 0) {
        // 发送数据，返回发送的字节数
        ssize_t sent = send(socketFd, data, remaining, 0);
        if (sent <= 0) {
            // 如果发送失败，则抛出异常
            throw MagicTrickException(socketError("发送网络数据失败"));
        }
        // 更新数据指针
        data += sent;
        // 更新剩余字节数
        remaining -= static_cast<size_t>(sent);
    }
}

// 接收单行消息
std::string NetworkGame::receiveLine(int socketFd) {
    // 创建一个字符串，用于保存接收到的消息
    std::string line;
    // 创建一个字符，用于保存接收到的字符
    char ch = '\0';
    // 当接收到的字符不为换行符时，继续接收

    while (true) {
        // 接收数据，返回接收的字节数
        ssize_t received = recv(socketFd, &ch, 1, 0);
        if (received <= 0) {
            // 如果接收失败，则抛出异常
            throw MagicTrickException(socketError("网络连接已关闭"));
        }
        // 如果接收到的字符为换行符，则退出循环
        if (ch == '\n') {
            // 退出循环
            break;
        }
        // 添加接收到的字符
        line.push_back(ch);
        // 如果消息长度超过4096字节，则抛出异常
        if (line.size() > 4096) {
            throw InvalidGameStateException("网络消息过长");
        }
    }

    // 去除字符串首尾空白字符，便于网络消息解析
    return trim(line);
}

// 发送牌堆信息给观众端
void NetworkGame::sendPiles(int socketFd,
                            int round,
                            const std::vector<std::vector<std::string>>& piles) {
    // 发送轮次信息给观众端，格式为 "ROUND|<round>"
    sendLine(socketFd, "ROUND|" + std::to_string(round));
    // 发送每堆牌的信息给观众端，格式为 "PILE|<pileNumber>|<cards>"
    for (int i = 0; i < 3; ++i) {
        sendLine(socketFd, "PILE|" + std::to_string(i + 1) + "|" + joinCards(piles[i]));
    }
    // 发送结束轮次信息给观众端，格式为 "ENDROUND"
    sendLine(socketFd, "ENDROUND");
}

// 接收牌堆信息给观众端
std::vector<std::vector<std::string>> NetworkGame::receivePiles(int socketFd, int& round) {
    // 接收轮次信息，格式为 "ROUND|<round>"
    std::string line = receiveLine(socketFd);
    // 如果轮次信息格式不正确，则抛出异常
    if (line.find("ROUND|") != 0) {
        throw InvalidGameStateException("未收到魔术师端发送的轮次消息");
    }

    // 解析轮次信息
    round = std::stoi(line.substr(6));
    // 创建一个三维数组，用于保存三堆牌
    std::vector<std::vector<std::string>> piles(3);
    // 当收到结束轮次信息时，退出循环
    while (true) {
        // 接收牌堆信息，格式为 "PILE|<pileNumber>|<cards>"
        line = receiveLine(socketFd);
        // 如果收到结束轮次信息，则退出循环
        if (line == "ENDROUND") {
            break;
        }
        // 如果牌堆信息格式不正确，则抛出异常
        if (line.find("PILE|") != 0) {
            throw InvalidGameStateException("未收到魔术师端发送的牌堆消息");
        }

        // 解析分隔符，PILE|2|[A♥],[3♣]
        size_t firstSep = line.find('|');
        size_t secondSep = line.find('|', firstSep + 1);
        if (secondSep == std::string::npos) {
            throw InvalidGameStateException("牌堆消息格式错误");
        }

        // 解析牌堆编号
        int pileNumber = std::stoi(line.substr(firstSep + 1, secondSep - firstSep - 1));
        if (pileNumber < 1 || pileNumber > 3) {
            throw InvalidGameStateException("远端牌堆编号超出范围");
        }
        // 解析牌堆中的牌，格式为 "[A♥],[3♣]"
        piles[pileNumber - 1] = splitCards(line.substr(secondSep + 1));
    }

    // 返回三堆牌
    return piles;
}

// 去除字符串首尾空白字符，便于网络消息解析
std::string NetworkGame::trim(const std::string& value) {
    // 创建一个变量，用于保存字符串起始位置
    size_t start = 0;
    // 当字符串起始位置小于字符串长度且字符串起始位置的字符为空白字符时，继续循环
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    // 创建一个变量，用于保存字符串结束位置
    size_t end = value.size();
    // 当字符串结束位置大于字符串起始位置且字符串结束位置的字符为空白字符时，继续循环
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    // 返回去除首尾空白字符后的字符串
    return value.substr(start, end - start);
}
