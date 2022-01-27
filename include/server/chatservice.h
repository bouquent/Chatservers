#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "public.h"
#include "deletecopy.h"
#include "json.hpp"
#include "server/db/redis.h"
#include "server/model/usermodel.h"
#include "server/model/offlinemessagemodel.h"
#include "server/model/friendmodel.h"
#include "server/model/groupmodel.h"

#include <mymuduo/tcpconnection.hpp>
#include <functional>
#include <mutex>
#include <map>

using namespace nlohmann;
using MsgHandler = std::function<void(const TcpConnectionPtr&, const json&, Timestamp)>;

class ChatService : deletecopy
{
public:

    static ChatService* getchatService(); 
    ~ChatService();

    MsgHandler getHandler(MSG_ID msgid);  //根据相应的事件类型调用相应的回调函数

    void clientCloseExption(const TcpConnectionPtr&);   //客户异常断开连接
    void serverCloseExption();

    //登录注册业务
    void login(const TcpConnectionPtr& conn, const json& js, Timestamp time);  
    void logout(const TcpConnectionPtr& conn, const json& js, Timestamp time); 
    void reg(const TcpConnectionPtr& conn, const json& js, Timestamp time);   
    
    //聊天业务
    void oneChat(const TcpConnectionPtr& conn, const json& js, Timestamp time); 
    void groupChat(const TcpConnectionPtr& conn, const json& js, Timestamp time); 
    
    //好友群组信息修改
    void addFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time); 
    void deleteFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time); 
    void createGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time);
    void addGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time);

    //接受到心跳包消息
    void reciveHeart(const TcpConnectionPtr& conn, const json& js, Timestamp time);

    //在redis中注册的回调消息，当通道中有消息发布时调用
    void handleRedisSubscribeMessage(int userid, const std::string& msg);
    //检测心跳消息，删除超时的用户
    void heartTest();

private:
    ChatService();  //单例模式，构造函数私有化

    std::unordered_map<MSG_ID, MsgHandler> msgHandlers_;
    std::unordered_map<int, TcpConnectionPtr> connections_;
    std::mutex mutex_; //保证connections线程安全
    std::unordered_map<TcpConnectionPtr, int> heartMap_;

    //操作各种表的类
    UserModel userModel_;
    OfflineMessageModel offlineMessageModel_;
    FriendModel friendModel_;  
    GroupModel groupModel_;

    //为redis派发命令的类
    Redis redis_;
};



#endif 