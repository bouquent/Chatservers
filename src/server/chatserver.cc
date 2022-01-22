#include "server/chatserver.h"
#include "server/chatservice.h"
#include "public.h"
#include "json.hpp"
#include <mymuduo/logging.hpp>

using namespace nlohmann;

ChatServer::ChatServer(EventLoop* loop, const InetAddr& localaddr, const std::string& nameArg)
            : server_(loop, localaddr, nameArg)
{
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this,
                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
ChatServer::~ChatServer()
{}

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    if (!conn->connected()) {
        ChatService::getchatService()->clientCloseExption(conn);
        LOG_INFO("one client is disconnect the server!");
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    std::string message = buffer->retrieveAllAsString();
    json js = json::parse(message);
    
    MSG_ID msgtype = js["msgid"];
    
    auto msgHandler = ChatService::getchatService()->getHandler(msgtype);
    
    msgHandler(conn, js, time);
}


void ChatServer::start()
{
    server_.setThreadNum(1);  //3个subloop，一个mainloop
    
    server_.start();
}