#ifndef CHATSERVER_H
#define CHATSERBER_H

#include "deletecopy.h"

#include <mymuduo/tcpserver.hpp>
#include <mymuduo/eventloop.hpp>

class ChatServer : deletecopy
{
public:
    ChatServer(EventLoop* loop, const InetAddr& localaddr, const std::string& nameArg = std::string());
    ~ChatServer();
   

    void onConnection(const TcpConnectionPtr& conn);  //有用户连接丶断开
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);     //有用户进行读写操作

    void start();

private:
    TcpServer server_;
};


#endif