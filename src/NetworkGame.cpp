#include "NetworkGame.h"
#include "Exceptions.h"
#include "Utils.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <random>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace {
const int BACKLOG = 1;
const int REVEAL_INDEX = 10;

std::string socketError(const std::string& action) {
    return action + ": " + std::strerror(errno);
}
}

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
        throw InvalidInputException("Port must be between 1024 and 65535");
    }

    SocketHandle server(socket(AF_INET, SOCK_STREAM, 0));
    if (!server.isValid()) {
        throw MagicTrickException(socketError("Failed to create server socket"));
    }

    int opt = 1;
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("Failed to bind server socket"));
    }

    if (listen(server.fd, BACKLOG) < 0) {
        throw MagicTrickException(socketError("Failed to listen on server socket"));
    }

    Utils::printColored("\n魔术师端已启动，等待观众连接...\n", Utils::COLOR_GREEN);
    Utils::printColored("Magician server listening on port " + std::to_string(port) + ".\n",
                        Utils::COLOR_GREEN);

    sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    SocketHandle client(accept(server.fd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength));
    if (!client.isValid()) {
        throw MagicTrickException(socketError("Failed to accept client"));
    }

    char clientIp[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));
    Utils::printColored("观众已连接 (Audience connected): ", Utils::COLOR_CYAN);
    std::cout << clientIp << "\n";

    sendLine(client.fd, "WELCOME|21 Card Network Duel");

    std::vector<std::string> deck = createDeck();
    shuffleDeck(deck);

    std::cout << "\n请让观众从收到的21张牌中记住一张。\n";
    sendLine(client.fd, "DECK|" + joinCards(deck));

    for (int round = 1; round <= 3; ++round) {
        auto piles = dealIntoPiles(deck);
        sendPiles(client.fd, round, piles);

        Utils::printTitle("网络对战 - 第 " + std::to_string(round) + " 轮");
        for (int i = 0; i < 3; ++i) {
            Utils::printColored("牌堆 " + std::to_string(i + 1) + ": ", Utils::COLOR_CYAN);
            std::cout << joinCards(piles[i]) << "\n";
        }
        std::cout << "等待观众选择牌堆...\n";

        std::string line = receiveLine(client.fd);
        if (line.find("CHOICE|") != 0) {
            throw InvalidGameStateException("Unexpected network message: " + line);
        }

        int chosenPile = std::stoi(line.substr(7));
        if (chosenPile < 1 || chosenPile > 3) {
            throw InvalidInputException("Remote pile choice out of range");
        }

        Utils::printColored("观众选择了牌堆 (Audience chose pile): ",
                            Utils::COLOR_YELLOW);
        std::cout << chosenPile << "\n";
        reorganize(deck, piles, chosenPile);
        sendLine(client.fd, "ACK|ROUND|" + std::to_string(round));
    }

    std::string revealCard = deck[REVEAL_INDEX];
    sendLine(client.fd, "REVEAL|" + revealCard);
    Utils::printTitle("网络揭晓 (Network Reveal)");
    Utils::printStyled("观众记住的牌应该是: " + revealCard + "\n",
                       Utils::COLOR_GREEN, Utils::BOLD);
    Utils::printColored("本局网络对战结束。(Network duel complete.)\n", Utils::COLOR_GREEN);
}

void NetworkGame::runAudienceClient(const std::string& host, int port) {
    if (host.empty()) {
        throw InvalidInputException("Host cannot be empty");
    }
    if (port < 1024 || port > 65535) {
        throw InvalidInputException("Port must be between 1024 and 65535");
    }

    SocketHandle client(socket(AF_INET, SOCK_STREAM, 0));
    if (!client.isValid()) {
        throw MagicTrickException(socketError("Failed to create client socket"));
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
        throw InvalidInputException("Host must be an IPv4 address, for example 127.0.0.1");
    }

    Utils::printColored("\n正在连接魔术师端...\n", Utils::COLOR_CYAN);
    if (connect(client.fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw MagicTrickException(socketError("Failed to connect to magician"));
    }

    std::string welcome = receiveLine(client.fd);
    if (welcome.find("WELCOME|") == 0) {
        Utils::printColored("连接成功 (Connected): ", Utils::COLOR_GREEN);
        std::cout << welcome.substr(8) << "\n";
    }

    std::string deckLine = receiveLine(client.fd);
    if (deckLine.find("DECK|") != 0) {
        throw InvalidGameStateException("Expected initial deck from magician");
    }

    std::cout << "\n请从这些牌中记住一张，但不要告诉魔术师：\n";
    std::cout << deckLine.substr(5) << "\n";
    Utils::pressAnyKey();

    for (int expectedRound = 1; expectedRound <= 3; ++expectedRound) {
        int round = 0;
        auto piles = receivePiles(client.fd, round);
        if (round != expectedRound) {
            throw InvalidGameStateException("Unexpected round number from magician");
        }

        Utils::printTitle("网络对战 - 第 " + std::to_string(round) + " 轮");
        for (int i = 0; i < 3; ++i) {
            Utils::printColored("牌堆 " + std::to_string(i + 1) + ": ", Utils::COLOR_CYAN);
            std::cout << joinCards(piles[i]) << "\n";
        }

        int choice = Utils::getIntInput("\n你的牌在哪一堆？(Which pile contains your card?): ", 1, 3);
        sendLine(client.fd, "CHOICE|" + std::to_string(choice));

        std::string ack = receiveLine(client.fd);
        if (ack.find("ACK|ROUND|") != 0) {
            throw InvalidGameStateException("Expected round acknowledgment from magician");
        }
    }

    std::string revealLine = receiveLine(client.fd);
    if (revealLine.find("REVEAL|") != 0) {
        throw InvalidGameStateException("Expected reveal from magician");
    }

    Utils::printTitle("网络揭晓 (Network Reveal)");
    Utils::printStyled("魔术师猜你的牌是: " + revealLine.substr(7) + "\n",
                       Utils::COLOR_GREEN, Utils::BOLD);
    Utils::confirm("魔术师猜对了吗？(Did the magician guess correctly?)");
}

std::vector<std::string> NetworkGame::createDeck() {
    const std::string suits[] = {"H", "D", "C"};
    const std::string ranks[] = {"A", "2", "3", "4", "5", "6", "7"};

    std::vector<std::string> deck;
    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            deck.push_back("[" + rank + suit + "]");
        }
    }
    return deck;
}

void NetworkGame::shuffleDeck(std::vector<std::string>& deck) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(deck.begin(), deck.end(), rng);
}

std::vector<std::vector<std::string>> NetworkGame::dealIntoPiles(const std::vector<std::string>& deck) {
    std::vector<std::vector<std::string>> piles(3);
    for (size_t i = 0; i < deck.size(); ++i) {
        piles[i % 3].push_back(deck[i]);
    }
    return piles;
}

void NetworkGame::reorganize(std::vector<std::string>& deck,
                             const std::vector<std::vector<std::string>>& piles,
                             int chosenPile) {
    int order[3] = {0, 1, 2};
    if (chosenPile == 1) {
        order[0] = 1;
        order[1] = 0;
        order[2] = 2;
    } else if (chosenPile == 3) {
        order[0] = 0;
        order[1] = 2;
        order[2] = 1;
    }

    deck.clear();
    for (int idx : order) {
        deck.insert(deck.end(), piles[idx].begin(), piles[idx].end());
    }
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
            throw MagicTrickException(socketError("Failed to send network data"));
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
            throw MagicTrickException(socketError("Network connection closed"));
        }
        if (ch == '\n') {
            break;
        }
        line.push_back(ch);
        if (line.size() > 4096) {
            throw InvalidGameStateException("Network message is too large");
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
        throw InvalidGameStateException("Expected round message from magician");
    }

    round = std::stoi(line.substr(6));
    std::vector<std::vector<std::string>> piles(3);
    while (true) {
        line = receiveLine(socketFd);
        if (line == "ENDROUND") {
            break;
        }
        if (line.find("PILE|") != 0) {
            throw InvalidGameStateException("Expected pile message from magician");
        }

        size_t firstSep = line.find('|');
        size_t secondSep = line.find('|', firstSep + 1);
        if (secondSep == std::string::npos) {
            throw InvalidGameStateException("Malformed pile message");
        }

        int pileNumber = std::stoi(line.substr(firstSep + 1, secondSep - firstSep - 1));
        if (pileNumber < 1 || pileNumber > 3) {
            throw InvalidGameStateException("Remote pile number out of range");
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
