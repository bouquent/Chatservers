#include "server/chatservice.h"
#include "server/model/user.h"
#include "json.hpp"

#include <iostream>
using namespace nlohmann;

ChatService* ChatService::getchatService()
{
    static ChatService service;
    return &service;
}


ChatService::ChatService()
{
    using namespace std::placeholders;
    msgHandlers_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1,  _2, _3)});
    msgHandlers_.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    msgHandlers_.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});
    msgHandlers_.insert({ONECHAT_MSG, std::bind(&ChatService::onechat, this, _1, _2, _3)});
}

ChatService::~ChatService() 
{}


MsgHandler ChatService::getHandler(MSG_ID msgid)
{
    if (msgHandlers_.find(msgid) == msgHandlers_.end()) {
        return [=](auto a, auto b, auto c) {
            std::cout << "migid :" << msgid << ", cannot find the msgid handler!" << std::endl;
        };
        
    } else {
        return msgHandlers_[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();   //获取id 
    std::string passwd = js["passwd"].get<std::string>();
    User user = userModel_.query(id);

    json response;
    response["msgid"] = LOGIN_ACK_MSG;
    if (passwd != user.passwd()) {      //密码错误
        response["errmsg"] = "your password is wrong!";
        response["errno"] = -1;
        conn->send(response.dump());
        return ;
    } 
    if (user.state() == online) {       //用户已经登陆过了
        response["errmsg"] = "此用户已经登录了!";
        response["errno"] = -1;
        conn->send(response.dump());
        return ;
    }

    connections_.insert({id, conn});
    userModel_.updateState(user, online);
    response["successmsg"] = "登录成功!";
    response["errno"] = 0;
    conn->send(response.dump());
    return ;
}

void ChatService::logout(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    bool findis = false;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = connections_.find(userid);
        if (iter != connections_.end()) {
            connections_.erase(userid);
            findis = true;
        }
    }

    if (findis) {
        User user;
        user.setId(userid);

        userModel_.updateState(user, offline);
        json response;
        response["errno"] = 0;
        response["msgid"] = LOGOUT_MSG;
        response["successmsg"] = "you logout sucess";
        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    std::string name = js["name"].get<std::string>();
    std::string passwd = js["passwd"].get<std::string>();
    User user;
    user.setName(name);
    user.setPasswd(passwd);
    userModel_.insert(user);

    json response;
    response["msgid"] = REG_ACK_MSG;

    if (user.id() == -1) {
        response["errno"] = -1;
        response["errmsg"] = "sorry, your apply is failure";
        conn->send(response.dump());
        return ;
    }

    response["errno"] = 0;
    response["id"] = user.id();
    response["successmsg"] = "this is your id, pealse remember it!"; 
    conn->send(response.dump());
}

void ChatService::onechat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = connections_.find(toid);
        if (iter != connections_.end()) {
            //说明这个人也在这个服务上连接着,直接将消息转发给他
            iter->second->send(js.dump());
            return ;
        }
    }


    //插入到离线消息中
    offlineMessageModel_.insert(toid, js.dump());
}

void ChatService::clientCloseExption(const TcpConnectionPtr& conn)
{

}