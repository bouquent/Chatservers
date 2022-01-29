#include "server/chatserver.h"
#include "server/chatservice.h"
#include "public.h"
#include "json.hpp"
#include "mylog.h"

#include <signal.h>
#include <unistd.h>

using namespace nlohmann;

static void sighandler(int signal)
{
    //服务器异常退出，捕获SIGINT信号
    ChatService::getchatService()->serverCloseExption();

    ::exit(0);
}


ChatServer::ChatServer(EventLoop* loop, const InetAddr& localaddr, const std::string& nameArg)
            : server_(loop, localaddr, nameArg)
            , loop_(loop)
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
        MYLOG_INFO("one client is disconnect the server!");
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    std::string message = buffer->retrieveAllAsString();
    if (!json::accept(message)) {
        MYLOG_INFO("recvice a message, but is not json!");
        return ;
    }
    
    //反序列化
    json js = json::parse(message);

    if (!js.contains("msgid")) {
         MYLOG_ERROR("revice a message, but it's not contains msgid");
         return ;
    }

    MSG_ID msgtype = (MSG_ID)js["msgid"].get<int>();
    
    auto msgHandler = ChatService::getchatService()->getHandler(msgtype);
    
    msgHandler(conn, js, time);
}


void ChatServer::start()
{
    server_.setThreadNum(2);  //1个subloop，一个mainloop
    
    server_.start();

    loop_->runEvery(30.0, std::bind(&ChatService::heartTest, ChatService::getchatService()));   //每隔30秒进行一次心跳检测

    signal(SIGINT, sighandler);
}