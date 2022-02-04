#include "client/clientservice.h"
#include "client/gettime.h"
#include "public.h"
#include "mylog.h"
#include "json.hpp"

#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <vector>



using nlohmann::json;

enum State
{
    offline = 0,
    online,
};

ClientService::ClientService()
{
    using namespace std::placeholders;
    userOffline_ = true;
    userid_ = -1;
    commandMap_.insert({"help", "显示所有功能 格式:help"});
    commandMap_.insert({"chat", "与好友私聊   格式:chat:friendid:msg"});
    commandMap_.insert({"groupchat", "群聊         格式:groupchat:groupid:msg"});

    commandMap_.insert({"addfriend",  "添加好友     格式:addfriend:friendid"});
    commandMap_.insert({"deletefriend", "删除好友     格式:deletefriend:friendid"});
    commandMap_.insert({"addgroup", "添加群组     格式:addgroup:groupid"});
    commandMap_.insert({"creategroup", "创建群组     格式:creategroup:groupname:groupdesc"});
    commandMap_.insert({"logout", "注销         格式:logout"});

    //注册业务回调函数
    commandHandlerMap_.insert({"help", std::bind(&ClientService::help, this, _1) });
    commandHandlerMap_.insert({"chat", std::bind(&ClientService::chat, this, _1) });
    commandHandlerMap_.insert({"groupchat", std::bind(&ClientService::groupChat, this, _1) });

    commandHandlerMap_.insert({"addfriend", std::bind(&ClientService::addFriend, this, _1) });
    commandHandlerMap_.insert({"deletefriend", std::bind(&ClientService::deleteFriend, this, _1) });
    commandHandlerMap_.insert({"addgroup", std::bind(&ClientService::addGroup, this, _1) });
    commandHandlerMap_.insert({"creategroup", std::bind(&ClientService::createGroup, this, _1) });
    commandHandlerMap_.insert({"logout", std::bind(&ClientService::logout, this, _1) });
}

ClientService::~ClientService()
{
    close(sockfd_);
}

ClientService* ClientService::getService() 
{
    static ClientService clientService;
    return &clientService;
}

bool ClientService::connect(const char* ip, uint16_t port)
{
    ::bzero(&serverAddr_, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = inet_addr(ip);
    serverAddr_.sin_port = htons(port);

    sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == -1) {
        MYLOG_ERROR("[%s]:%d create socket error!", __FILE__, __LINE__);
        return false;
    }

    int ret = ::connect(sockfd_, (struct sockaddr*)& serverAddr_, sizeof(serverAddr_));
    if (ret != 0) {
        MYLOG_ERROR("[%s]:%d connect wrong!", __FILE__, __LINE__);
        return false;
    }
    return true;
}

void ClientService::readMessageTask(ClientService* service)
{
    while (!service->userOffline_) {
        char buffer[4096] = {0};
        int len = ::recv(service->sockfd_, buffer, sizeof(buffer), 0);
        if (len == -1) {
            MYLOG_ERROR("[%s]:%d recv error!", __FILE__, __LINE__);
            continue;
        } else if (len == 0) {
             MYLOG_ERROR("sorry, server is close exption!");
            service->userOffline_ = false;
            exit(0);
        }

        if (!json::accept(buffer)) {
            MYLOG_INFO("receive a message, but is not json!")
            continue;
        }
        json jsrecv = json::parse(buffer);

        if (ONE_CHAT_MSG == jsrecv["msgid"].get<int>()) {
            std::cout << jsrecv["time"].get<std::string>() << "[" << jsrecv["userid"].get<int>() << "] " 
                      << jsrecv["name"].get<std::string>() << " said: " << jsrecv["message"].get<std::string>() 
                      << std::endl; 

        } else if (GROUP_CHAT_MSG == jsrecv["msgid"].get<int>()) {
            std::cout << "群消息[" << jsrecv["groupid"].get<int>() << "]:\n" << jsrecv["time"].get<std::string>() << "[" 
                    << jsrecv["userid"].get<int>() << "] " << jsrecv["name"].get<std::string>() 
                    << " said: " << jsrecv["message"].get<std::string>() << std::endl; 

        } else if (LOGOUT_MSG == jsrecv["msgid"].get<int>()) {
            //退出登录请求服务已处理管理，关闭本地业务
            service->userOffline_ = true;
            service->userid_ = -1; 
            std::vector<std::string>().swap(service->g_currentFriend);
            std::vector<std::string>().swap(service->g_currentGroup);
        } else if (ACK_MSG == jsrecv["msgid"].get<int>()) {
            if (jsrecv["errno"] == -1) {
                //出错的回执消息
                std::cout << jsrecv["errmsg"].get<std::string>() << std::endl;
            } else {
                //显示服务器的回应消息
                std::cout << "系统消息:" << jsrecv["successmsg"].get<std::string>() << std::endl;
            }
        }
    }
}


void ClientService::sendHeartMsg(ClientService* service)
{
    while (!service->userOffline_) {
        sleep(30);          //模拟定时效果
        json js;
        js["msgid"] = HEART_MSG;
        std::string heartmsg = js.dump();
        int ret = ::send(service->sockfd_, heartmsg.c_str(), heartmsg.size(), 0);
        if (ret == -1 && errno != EINTR) {
            MYLOG_ERROR("[%s]:%d send heart message failure!", __FILE__, __LINE__);
        }
    }
} 

//注册业务
bool ClientService::regNewUser(const std::string& name, const std::string& passwd)
{
    json js;
    js["msgid"] = 3;
    js["name"] = name;
    js["passwd"] = passwd;
    
    std::string request = js.dump();
    int ret = ::send(sockfd_, request.c_str(), request.size(), 0);
    if (ret == -1) {
       MYLOG_ERROR("[%s]:%d sorry, register user application send failure!", __FILE__, __LINE__);
       return false;
    }

    //接受返回的数据
    char buffer[4096] = {0};
    int len = ::recv(sockfd_, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        MYLOG_ERROR("[%s]:%d sorry, register user recv message error!", __FILE__, __LINE__)
        return false;
    } 

    json jsrev = json::parse(buffer);
    if (jsrev["errno"] != 0) {
        std::cout << jsrev["errmsg"] << std::endl;
        return false;
    }
    int userid = jsrev["newid"].get<int>();
    std::cout << "this is your id: "<< userid << ", please don't remember it!" << std::endl;
    return true;
}  

//登录业务
bool ClientService::loginUser(int userid, const std::string& passwd)
{
    json js;
    js["msgid"] = LOGIN_MSG;
    js["userid"] = userid;
    js["passwd"] = passwd;

    std::string request = js.dump();
    int ret = ::send(sockfd_, request.c_str(), request.size(), 0);
    if (ret == -1) {
        MYLOG_ERROR("[%s]:%d sorry, login user application send failure!", __FILE__, __LINE__)
        return false;
    }

    //接受返回的数据
    char buffer[4096] = {0};
    int len = ::recv(sockfd_, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        MYLOG_ERROR("[%s]:%d sorry, login user recv message error!", __FILE__, __LINE__)
        return false;
    } 

    json jsrev = json::parse(buffer);
    if (jsrev["errno"].get<int>() != 0) {
        std::cout << jsrev["errmsg"].get<std::string>() << std::endl;
        return false;
    } 

    //登录成功展示好友列表
    if (jsrev.contains("friends")) {
        std::vector<std::string> friendsMessage = jsrev["friends"];
        std::cout << "--------------Friend List---------------------" << std::endl;
        for (auto friendUser : friendsMessage) {
            json jsUser = json::parse(friendUser);
            std::string state = "offline";
            if (jsUser["state"].get<int>() == online) state = "online";
            std::cout << jsUser["userid"].get<int>() << " " << jsUser["name"].get<std::string>() << " " << state << std::endl;
        }
        std::cout << "\n\n";
        g_currentFriend.clear();
        g_currentFriend = friendsMessage;
    }

    //展示群组列表
    if (jsrev.contains("groups")) {
        std::cout << "--------------Group List---------------------" << std::endl;
        std::vector<std::string> groupsMessage = jsrev["groups"];
        for (auto groupMessage : groupsMessage) {
            json jsgroup = json::parse(groupMessage);
            std::cout << jsgroup["groupid"].get<int>() << " " << jsgroup["groupname"].get<std::string>() 
                    << " " << jsgroup["groupdesc"].get<std::string>() << std::endl;

            std::vector<std::string> groupUsersMessage = jsgroup["groupUsers"];
            for (auto groupUser : groupUsersMessage) {
                json jsuser = json::parse(groupUser);
                std::string state = "offline";
                if (jsuser["state"] == online) state = "online";
                std::cout << "    "<< jsuser["userid"].get<int>() << "   " << jsuser["name"].get<std::string>() 
                    << "   " << jsuser["role"].get<std::string>() << "   " << state << std::endl;
            }
        }
        std::cout << "\n\n";
        g_currentGroup.clear();
        g_currentGroup = groupsMessage;
    }

    //展示离线消息
    if (jsrev.contains("offlinemessage")) {
        std::cout << "-----------your offlineMessage------------" << std::endl;
        std::vector<std::string> messages = jsrev["offlinemessage"];
        for (auto message : messages) {
            auto jsmsg = json::parse(message);
            if (jsmsg["msgid"].get<int>() == ONE_CHAT_MSG) {
                std::cout << jsmsg["time"].get<std::string>() << "[" << jsmsg["userid"].get<int>() << "] " 
                << jsmsg["name"].get<std::string>() << " said: " << jsmsg["message"].get<std::string>() << std::endl; 

            } else if (jsmsg["msgid"] == GROUP_CHAT_MSG) {
                std::cout << "群消息[" << jsmsg["groupid"].get<int>() << "]:\n" << jsmsg["time"].get<std::string>() 
                        << "[" << jsmsg["userid"].get<int>() << "] " << jsmsg["name"].get<std::string>() << " said: " 
                        << jsmsg["message"].get<std::string>() << std::endl; 
            } 
            std::cout << "\n\n";   
        }
    }
    
    //填写当前用户信息
    userid_ = jsrev["yourid"].get<int>();
    userName_ = jsrev["yourname"].get<std::string>(); 
    userOffline_ = false;
    return true;
}


void ClientService::mainMenu()
{
    this->help();
    while (!userOffline_) {
        std::string request;
        getline(std::cin, request);
        int idx = request.find(":");
        std::string command = request.substr(0, idx);
        std::string msg = request.substr(idx + 1, request.size() - idx - 1);

        auto iter = commandHandlerMap_.find(command);
        if (iter == commandHandlerMap_.end()) {
            std::cout << "the command is invaild!" << std::endl;
            continue;
        }
        iter->second(msg);
        
        if (command == "logout") {
            //如果发送的了logout请求，那么不能再接受用户的请求
            break;
        }
    }
}

//给登录成功的用户提供的业务功能
void ClientService::help(const std::string& s)
{
    std::cout << "<<<<<<<<<<<<<<<<<<<show command list>>>>>>>>>>>>>>" << std::endl;
    for (auto command : commandMap_) {
        std::cout << command.second << std::endl;
    }
}

void ClientService::chat(const std::string& msg)
{
    assert(userid_ != -1); //用户此时一定是处于登录状态

    int idx = msg.find(":");
    if (idx == -1) {
        MYLOG_ERROR("invaild input!");
        return ;
    }
    int friendid = 0;
    friendid = atoi(msg.substr(0, idx).c_str());                    //获取好友id号
    std::string message = msg.substr(idx + 1, msg.size() - idx - 1);    //获取发送的消息 

    json jssend;
    jssend["msgid"] = ONE_CHAT_MSG;
    jssend["userid"] = userid_;
    jssend["name"] = userName_;
    jssend["toid"] = friendid;
    jssend["time"] = GetTime::now().toString();
    jssend["message"] = message;
    
    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send chat msg failure->%s", message.c_str());
    } 
}


void ClientService::groupChat(const std::string& msg)
{
    assert(userid_ != -1); //用户此时一定是处于登录状态

    int idx = msg.find(":");
    if (idx == -1) {
        MYLOG_ERROR("invaild input!");
        return ;
    }
    int groupid = atoi(msg.substr(0, idx).c_str());                    //获取群组id号
    std::string message = msg.substr(idx + 1, msg.size() - idx - 1);    //获取发送的消息 

    json jssend;
    jssend["msgid"] = GROUP_CHAT_MSG;
    jssend["userid"] = userid_;
    jssend["name"] = userName_;
    jssend["groupid"] = groupid;
    jssend["time"] = GetTime::now().toString();
    jssend["message"] = message;
    
    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
          MYLOG_ERROR("sorry, send groupchat msg failure->%s", message.c_str());
    } 
}

void ClientService::addFriend(const std::string& msg)
{
    assert(userid_ != -1); //用户此时一定是处于登录状态

    int friendid = atoi(msg.c_str());                    //获取好友id号

    json jssend;
    jssend["msgid"] = ADD_FRIEND_MSG;
    jssend["userid"] = userid_;
    jssend["friendid"] = friendid;
    jssend["time"] = GetTime::now().toString();
    
    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send addFriend msg failure, please try once again!");
    } 
}

void ClientService::deleteFriend(const std::string& msg)
{
    assert(userid_ != -1); //用户此时一定是处于登录状态

    int friendid = atoi(msg.c_str());                    //获取好友id号

    json jssend;
    jssend["msgid"] = DELETE_FRIEND_MSG;
    jssend["userid"] = userid_;
    jssend["friendid"] = friendid;
    jssend["time"] = GetTime::now().toString();
    
    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send deleteFriend msg failure, please try once again!");
    } 
}

void ClientService::addGroup(const std::string& msg)
{
    assert(userid_ != -1); //用户此时一定是处于登录状态

    int groupid = atoi(msg.c_str());

    json jssend;
    jssend["msgid"] = ADD_GROUP;
    jssend["userid"] = userid_;
    jssend["groupid"] = groupid;
    jssend["time"] = GetTime::now().toString();
    
    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send addGroup msg failure, please try once again!");
    } 
}

void ClientService::createGroup(const std::string& msg)
{
    assert(userid_ != -1);

    int idx = msg.find(":");
    std::string groupName = msg.substr(0, idx);
    std::string groupDesc = msg.substr(idx + 1, msg.size() - idx - 1);

    json jssend;
    jssend["msgid"] = CREATE_GROUP;
    jssend["userid"] = userid_;
    jssend["groupName"] = groupName;
    jssend["groupDesc"] = groupDesc;
    jssend["time"] = GetTime::now().toString();

    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send createGroup msg failure, please try once again!");
    } 
}

void ClientService::logout(const std::string& msg)
{
    assert(userid_ != -1);

    json jssend;
    jssend["msgid"] = LOGOUT_MSG;
    jssend["userid"] = userid_;

    std::string sendmsg = jssend.dump();
    int ret = ::send(sockfd_, sendmsg.c_str(), sendmsg.size(), 0);
    if (-1 == ret) {
        MYLOG_ERROR("sorry, send createGroup msg failure, please try once again!");
    }
}