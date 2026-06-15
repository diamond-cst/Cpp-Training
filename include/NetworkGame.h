#ifndef NETWORKGAME_H
#define NETWORKGAME_H

#include <string>
#include <vector>

// 网络双人对战 (Network two-player game)
// 魔术师作为服务端，观众作为客户端，通过socket同步牌堆和选择。
class NetworkGame {
    friend class NetworkMagicianTrick;

public:
    static void runMagicianServer(int port);
    static void runAudienceClient(const std::string& host, int port);

private:
    struct SocketHandle {
        int fd;
        SocketHandle();
        explicit SocketHandle(int socketFd);
        ~SocketHandle();
        SocketHandle(const SocketHandle&) = delete;
        SocketHandle& operator=(const SocketHandle&) = delete;
        SocketHandle(SocketHandle&& other) noexcept;
        SocketHandle& operator=(SocketHandle&& other) noexcept;
        bool isValid() const;
        int release();
        void reset(int socketFd = -1);
    };

    static std::string joinCards(const std::vector<std::string>& cards);
    static std::vector<std::string> splitCards(const std::string& cards);

    static void sendLine(int socketFd, const std::string& line);
    static std::string receiveLine(int socketFd);
    static void sendPiles(int socketFd,
                          int round,
                          const std::vector<std::vector<std::string>>& piles);
    static std::vector<std::vector<std::string>> receivePiles(int socketFd, int& round);

    static std::string trim(const std::string& value);
};

#endif // NETWORKGAME_H
