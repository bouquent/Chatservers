#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include "deletecopy.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <unistd.h>
#include <netinet/in.h>

class ClientService : deletecopy
{
public:
    static ClientService* getService(); //外部接口
    static void readMessageTask(ClientService* service);      //专门读取并解读外部发送来的消息
    static void sendHeartMsg(ClientService* service);         //专门发送心跳包的线程调用

    bool connect();
    void mainEnum(); 

    //登录注册功能
    bool regNewUser(const std::string& name, const std::string& passwd);  
    bool loginUser(int userid, const std::string& passwd);

    //用户登录后的业务功能
    void mainMenu();
    void help(const std::string& msg = "");
    void chat(const std::string& msg);
    void groupChat(const std::string& msg);

    void addFriend(const std::string& msg);
    void deleteFriend(const std::string& msg);
    void addGroup(const std::string& msg);
    void createGroup(const std::string& msg);
    void logout(const std::string& msg = "");

    //用户信息的获取和修改
    bool userLogin() const {return userOffline_; }
    std::string name() const { return userName_; }
    void setName(const std::string& name) { userName_ = name; }
    int sockfd() const { return sockfd_; }
    struct sockaddr_in clientAddr() const { return serverAddr_; }
    void setSockfd(int sockfd) { sockfd_ = sockfd; }
    void setClientAddr(struct sockaddr_in sockaddr) { serverAddr_ = sockaddr; }
private:
    ClientService(); //单例模式，构造函数私有化
    using HandlerRequest = std::function<void(std::string)>;

private:
    std::unordered_map<std::string, std::string> commandMap_;   //支持的命令列表
    std::unordered_map<std::string, HandlerRequest> commandHandlerMap_;

    bool userOffline_;                              //用户是否上线
    int userid_;                                  //用户自己的id
    std::string userName_;                             //用户的名字
    std::vector<std::string> g_currentFriend;     //当前好友信息
    std::vector<std::string> g_currentGroup;      //当前群组信息

    struct sockaddr_in serverAddr_;
    int sockfd_;
};



#endif