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
const int BACKLOG = 1;

std::string socketError(const std::string& action) {
    return action + ": " + std::strerror(errno);
}

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
    int clientFd;

    std::vector<std::vector<std::string>> currentPilesAsText() const {
        std::vector<std::vector<std::string>> piles(3);
        const Deck<Card>* sourcePiles[] = {&pile1, &pile2, &pile3};
        for (int pileIndex = 0; pileIndex < 3; ++pileIndex) {
            for (int i = 0; i < sourcePiles[pileIndex]->getSize(); ++i) {
                piles[pileIndex].push_back(sourcePiles[pileIndex]->getCard(i).toString(useColors));
            }
        }
        return piles;
    }

    std::string workingDeckToString() const {
        std::vector<std::string> cards;
        for (int i = 0; i < workingDeck.getSize(); ++i) {
            cards.push_back(workingDeck.getCard(i).toString(useColors));
        }
        return NetworkGame::joinCards(cards);
    }

protected:
    PileChoice requestAudienceChoice(int roundNumber) override {
        NetworkGame::sendPiles(clientFd, roundNumber, currentPilesAsText());
        Utils::printColored("\n等待观众选择牌堆...\n",
                            Utils::COLOR_CYAN);

        std::string line = NetworkGame::receiveLine(clientFd);
        if (line.find("CHOICE|") != 0) {
            throw InvalidGameStateException("收到异常网络消息: " + line);
        }

        int chosenPile = std::stoi(line.substr(7));
        if (chosenPile < 1 || chosenPile > 3) {
            throw InvalidInputException("观众选择的牌堆超出范围");
        }

        Utils::printColored("观众选择了第 " + std::to_string(chosenPile) + " 堆。\n",
                            Utils::COLOR_YELLOW);
        NetworkGame::sendLine(clientFd, "ACK|ROUND|" + std::to_string(roundNumber));
        return PileChoice(chosenPile);
    }

    bool confirmAudienceReveal(const Card& revealedCard) override {
        NetworkGame::sendLine(clientFd, "REVEAL|" + revealedCard.toString(useColors));

        std::string line = NetworkGame::receiveLine(clientFd);
        if (line.find("CONFIRM|") != 0) {
            throw InvalidGameStateException("未收到观众端的揭晓确认");
        }

        char result = line.size() > 8 ? line[8] : 'N';
        return result == 'Y' || result == 'y';
    }

public:
    explicit NetworkMagicianTrick(int socketFd)
        : TwentyOneCardTrick(true), clientFd(socketFd) {}

    void initialize() override {
        TwentyOneCardTrick::initialize();
        NetworkGame::sendLine(clientFd, "DECK|" + workingDeckToString());
    }
};

NetworkGame::SocketHandle::SocketHandle() : fd(-1) {}

NetworkGame::SocketHandle::SocketHandle(int socketFd) : fd(socketFd) {}

NetworkGame::SocketHandle::~SocketHandle() {
    reset();
}

NetworkGame::SocketHandle::SocketHandle(SocketHandle&& other) noexcept : fd(other.fd) {
    other.fd = -1;
}

NetworkGame::SocketHandle& NetworkGame::SocketHandle::operator=(SocketHandle&& other) noexcept {
    if (this != &other) {
        reset();
        fd = other.fd;
        other.fd = -1;
    }
    return *this;
}

bool NetworkGame::SocketHandle::isValid() const {
    return fd >= 0;
}

int NetworkGame::SocketHandle::release() {
    int value = fd;
    fd = -1;
    return value;
}

void NetworkGame::SocketHandle::reset(int socketFd) {
    if (fd >= 0) {
        close(fd);
    }
    fd = socketFd;
}

void NetworkGame::runMagicianServer(int port) {
    if (port < 1024 || port > 65535) {
        throw InvalidInputException("端口号必须在1024到65535之间");
    }

    SocketHandle server(socket(AF_INET, SOCK_STREAM, 0));
    if (!server.isValid()) {
        throw MagicTrickException(socketError("创建服务器套接字失败"));
    }

    int opt = 1;
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("绑定服务器端口失败"));
    }

    if (listen(server.fd, BACKLOG) < 0) {
        throw MagicTrickException(socketError("监听服务器端口失败"));
    }

    Utils::clearScreen();
    Utils::printTitle("21张牌魔术");
    Utils::printColored("\n等待观众进入房间...\n", Utils::COLOR_GREEN);
    Utils::printColored("房间端口: " + std::to_string(port) + "\n",
                        Utils::COLOR_CYAN);

    sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    SocketHandle client(accept(server.fd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength));
    if (!client.isValid()) {
        throw MagicTrickException(socketError("接收观众连接失败"));
    }

    char clientIp[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));
    Utils::printColored("观众已加入，游戏即将开始。\n", Utils::COLOR_GREEN);
    Utils::printColored("连接来源: ", Utils::COLOR_CYAN);
    std::cout << clientIp << "\n";

    sendLine(client.fd, "WELCOME|21张牌网络对战");

    NetworkMagicianTrick trick(client.fd);
    trick.setPlayerName("网络魔术师");
    trick.setUseAnimation(true);
    trick.setSoundEnabled(true);
    trick.initialize();

    while (!trick.isComplete()) {
        trick.performRound();
    }

    trick.reveal();
    Utils::printColored("本局双人魔术结束。\n", Utils::COLOR_GREEN);
}

void NetworkGame::runAudienceClient(const std::string& host, int port) {
    if (host.empty()) {
        throw InvalidInputException("主机地址不能为空");
    }
    if (port < 1024 || port > 65535) {
        throw InvalidInputException("端口号必须在1024到65535之间");
    }

    SocketHandle client(socket(AF_INET, SOCK_STREAM, 0));
    if (!client.isValid()) {
        throw MagicTrickException(socketError("创建客户端套接字失败"));
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
        throw InvalidInputException("主机地址必须是IPv4格式，例如127.0.0.1");
    }

    Utils::printColored("\n正在进入魔术师房间...\n", Utils::COLOR_CYAN);
    if (connect(client.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("连接魔术师端失败"));
    }

    std::string welcome = receiveLine(client.fd);
    if (welcome.find("WELCOME|") == 0) {
        Utils::printColored("已进入房间，等待魔术师发牌...\n", Utils::COLOR_GREEN);
    }

    std::string deckLine = receiveLine(client.fd);
    if (deckLine.find("DECK|") != 0) {
        throw InvalidGameStateException("未收到魔术师端发送的初始牌组");
    }

    displayAudienceInitialState(splitCards(deckLine.substr(5)));
    Utils::pressAnyKey();

    for (int expectedRound = 1; expectedRound <= 3; ++expectedRound) {
        int round = 0;
        auto piles = receivePiles(client.fd, round);
        if (round != expectedRound) {
            throw InvalidGameStateException("魔术师端发送的轮次不正确");
        }

        displayAudiencePiles(round, piles);

        int choice = Utils::getIntInput("\n你记住的牌在哪一堆？请选择: ", 1, 3);
        sendLine(client.fd, "CHOICE|" + std::to_string(choice));

        std::string ack = receiveLine(client.fd);
        if (ack.find("ACK|ROUND|") != 0) {
            throw InvalidGameStateException("未收到魔术师端的本轮确认");
        }
    }

    std::string revealLine = receiveLine(client.fd);
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
    sendLine(client.fd, std::string("CONFIRM|") + (correct ? "Y" : "N"));
}

std::string NetworkGame::joinCards(const std::vector<std::string>& cards) {
    std::ostringstream oss;
    for (size_t i = 0; i < cards.size(); ++i) {
        if (i > 0) {
            oss << " ";
        }
        oss << cards[i];
    }
    return oss.str();
}

std::vector<std::string> NetworkGame::splitCards(const std::string& cards) {
    std::vector<std::string> result;
    std::istringstream iss(cards);
    std::string card;
    while (iss >> card) {
        result.push_back(card);
    }
    return result;
}

void NetworkGame::sendLine(int socketFd, const std::string& line) {
    std::string payload = line + "\n";
    const char* data = payload.c_str();
    size_t remaining = payload.size();

    while (remaining > 0) {
        ssize_t sent = send(socketFd, data, remaining, 0);
        if (sent <= 0) {
            throw MagicTrickException(socketError("发送网络数据失败"));
        }
        data += sent;
        remaining -= static_cast<size_t>(sent);
    }
}

std::string NetworkGame::receiveLine(int socketFd) {
    std::string line;
    char ch = '\0';

    while (true) {
        ssize_t received = recv(socketFd, &ch, 1, 0);
        if (received <= 0) {
            throw MagicTrickException(socketError("网络连接已关闭"));
        }
        if (ch == '\n') {
            break;
        }
        line.push_back(ch);
        if (line.size() > 4096) {
            throw InvalidGameStateException("网络消息过长");
        }
    }

    return trim(line);
}

void NetworkGame::sendPiles(int socketFd,
                            int round,
                            const std::vector<std::vector<std::string>>& piles) {
    sendLine(socketFd, "ROUND|" + std::to_string(round));
    for (int i = 0; i < 3; ++i) {
        sendLine(socketFd, "PILE|" + std::to_string(i + 1) + "|" + joinCards(piles[i]));
    }
    sendLine(socketFd, "ENDROUND");
}

std::vector<std::vector<std::string>> NetworkGame::receivePiles(int socketFd, int& round) {
    std::string line = receiveLine(socketFd);
    if (line.find("ROUND|") != 0) {
        throw InvalidGameStateException("未收到魔术师端发送的轮次消息");
    }

    round = std::stoi(line.substr(6));
    std::vector<std::vector<std::string>> piles(3);
    while (true) {
        line = receiveLine(socketFd);
        if (line == "ENDROUND") {
            break;
        }
        if (line.find("PILE|") != 0) {
            throw InvalidGameStateException("未收到魔术师端发送的牌堆消息");
        }

        size_t firstSep = line.find('|');
        size_t secondSep = line.find('|', firstSep + 1);
        if (secondSep == std::string::npos) {
            throw InvalidGameStateException("牌堆消息格式错误");
        }

        int pileNumber = std::stoi(line.substr(firstSep + 1, secondSep - firstSep - 1));
        if (pileNumber < 1 || pileNumber > 3) {
            throw InvalidGameStateException("远端牌堆编号超出范围");
        }
        piles[pileNumber - 1] = splitCards(line.substr(secondSep + 1));
    }

    return piles;
}

std::string NetworkGame::trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}
