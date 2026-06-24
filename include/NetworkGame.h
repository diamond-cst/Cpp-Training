#ifndef NETWORKGAME_H
#define NETWORKGAME_H

#include <string>
#include <vector>

// 网络双人对战 (Network two-player game)
// 魔术师作为服务端，观众作为客户端，通过socket同步牌堆和选择。
class NetworkGame {
    friend class NetworkMagicianTrick;

public:
    // 运行魔术师服务器，监听指定端口，等待观众连接
    static void runMagicianServer(int port);
    // 运行观众客户端，连接指定主机和端口
    static void runAudienceClient(const std::string& host, int port);

private:
    struct SocketHandle {
        int fd; // socket文件描述符，自动管理 socket 生命周期，避免忘记关闭连接
        SocketHandle();
        explicit SocketHandle(int socketFd);
        ~SocketHandle();
        SocketHandle(const SocketHandle&) = delete; // 禁止拷贝构造，避免重复关闭同一个socket
        SocketHandle& operator=(const SocketHandle&) = delete; // 禁止拷贝赋值，避免重复关闭同一个socket
        SocketHandle(SocketHandle&& other) noexcept;    // 允许移动构造，转移socket所有权
        SocketHandle& operator=(SocketHandle&& other) noexcept;
        bool isValid() const;   // 检查socket是否有效
        int release();  // 释放socket所有权，返回fd并将fd置为-1，避免析构时关闭
        void reset(int socketFd = -1);  // 重置socket，关闭旧的fd并设置新的fd，默认关闭当前socket
    };

    // 将牌堆列表拼接为单行字符串，便于网络传输
    static std::string joinCards(const std::vector<std::string>& cards);
    // 将单行字符串拆分为牌堆列表，便于网络传输
    static std::vector<std::string> splitCards(const std::string& cards);

    // 发送和接收单行消息
    static void sendLine(int socketFd, const std::string& line);
    static std::string receiveLine(int socketFd);
    // 发送牌堆信息给观众端
    static void sendPiles(int socketFd,
                          int round,
                          const std::vector<std::vector<std::string>>& piles);
    // 接收观众端发送的牌堆信息
    static std::vector<std::vector<std::string>> receivePiles(int socketFd, int& round);

    // 去除字符串首尾空白字符，便于网络消息解析
    static std::string trim(const std::string& value);
};

#endif // NETWORKGAME_H
