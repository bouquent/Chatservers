#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "public.h"
#include "deletecopy.h"
#include "json.hpp"
#include "server/model/usermodel.h"
#include "server/model/offlinemessagemodel.h"

#include <mymuduo/tcpconnection.hpp>
#include <functional>
#include <mutex>
#include <map>

using namespace nlohmann;
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ChatService : deletecopy
{
public:

    static ChatService* getchatService(); 
    ~ChatService();

    MsgHandler getHandler(MSG_ID msgid);  //根据相应的事件类型调用相应的回调函数

    void clientCloseExption(const TcpConnectionPtr&);   //客户异常断开连接

    //业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);  //登录
    void logout(const TcpConnectionPtr& conn,  json& js, Timestamp time); //退出登录
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);    //注册
    void onechat(const TcpConnectionPtr& conn,  json& js, Timestamp time); //个人聊天

private:
    ChatService();  //单例模式，构造函数私有化m

    std::unordered_map<MSG_ID, MsgHandler> msgHandlers_;
    std::unordered_map<int, TcpConnectionPtr> connections_;
    std::mutex mutex_; //保证connections线程安全

    UserModel userModel_;
    OfflineMessageModel offlineMessageModel_;
};


#endif 