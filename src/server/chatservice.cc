#include "server/chatservice.h"
#include "server/model/user.h"
#include "server/model/groupuser.h"
#include "json.hpp"
#include "mylog.h"


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

    msgHandlers_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    msgHandlers_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    msgHandlers_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    msgHandlers_.insert({DELETE_FRIEND_MSG, std::bind(&ChatService::deleteFriend, this, _1, _2, _3)});
    msgHandlers_.insert({CREATE_GROUP, std::bind(&ChatService::createGroup, this, _1, _2, _3)});

    msgHandlers_.insert({HEART_MSG, std::bind(&ChatService::reciveHeart, this, _1, _2, _3)});

    if (redis_.connect()) {
        redis_.setNotify_message_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

ChatService::~ChatService() 
{
    for (auto iter : connections_) {
        userModel_.resetState(iter.first);
        redis_.unsubscribe(iter.first);
        iter.second->shutDown();
    }
}


MsgHandler ChatService::getHandler(MSG_ID msgid)
{
    if (msgHandlers_.find(msgid) == msgHandlers_.end()) {
        return [=](auto a, auto b, auto c) {
            MYLOG_INFO("msgid: %d, cannot find the msgid handler!", msgid);
        };
        
    } else {
        return msgHandlers_[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();   //获取id 
    std::string passwd = js["passwd"].get<std::string>();
    User user = userModel_.query(userid);

    json response;
    response["msgid"] = LOGIN_ACK_MSG;
    //检测登录是否符合要求
    if (passwd != user.passwd()) {      
        //密码错误
        response["errmsg"] = "your password is wrong!";
        response["errno"] = -1;
        conn->send(response.dump());
        return ;
    } 
    if (user.state() == online) {       
        //用户已经登陆过了
        response["errmsg"] = "此用户已经登录了!";
        response["errno"] = -1;
        conn->send(response.dump());
        return ;
    }

    //根据userid为用户订阅一个通道
    redis_.subscribe(userid);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.insert({userid, conn});
    }
    heartMap_.insert({conn, 0});            //将用户加入到心跳包检测机制中
    userModel_.updateState(user, online);
    response["successmsg"] = "登录成功!";
    response["errno"] = 0;
    response["yourid"] = user.id();
    response["yourname"] = user.name();

    //查询离线消息
    std::vector<std::string> offlinemessage = offlineMessageModel_.query(userid);
    if (!offlinemessage.empty()) {
        response["offlinemessage"] = offlinemessage;
        offlineMessageModel_.remove(userid);
    }

    //查询好友
    std::vector<User> friends_ = friendModel_.queryFriend(userid);
    if(!friends_.empty()) {
        std::vector<std::string> friendsmessage;
        for (auto user : friends_) {
            json jsaddfriends;
            jsaddfriends["userid"] = user.id();
            jsaddfriends["name"] = user.name();
            jsaddfriends["state"] = user.state();
            friendsmessage.push_back(jsaddfriends.dump());
        }
        response["friends"] = friendsmessage;
    }

    //查询群组
    std::vector<Group> groups = groupModel_.queryGroup(userid);
    if (!groups.empty()) {
        std::vector<std::string> groupsMessage;
        for (auto group : groups) {
            json jsgroup;
            jsgroup["groupid"] = group.groupId();
            jsgroup["groupname"] = group.groupName();
            jsgroup["groupdesc"] = group.groupDesc();
            std::vector<GroupUser> groupUsers = groupModel_.queryGroupUser(group.groupId());
            if (groupUsers.empty()) continue;
            std::vector<std::string> groupUsersMessage;
            for (auto groupuser : groupUsers) {
                json jsgroupUser;
                jsgroupUser["userid"] = groupuser.id();
                jsgroupUser["name"] = groupuser.name();
                jsgroupUser["role"] = groupuser.groupRole();
                jsgroupUser["state"] = groupuser.state();

                groupUsersMessage.push_back(jsgroupUser.dump());
            }
            jsgroup["groupUsers"] = groupUsersMessage;
            groupsMessage.push_back(jsgroup.dump());
        }
        response["groups"] = groupsMessage;
    }

    conn->send(response.dump());
    return ;
}

void ChatService::logout(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    bool findis = false;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = connections_.find(userid);
        if (iter != connections_.end()) {
            connections_.erase(userid);
            findis = true;
        }
    }

    json response;
    response["msgid"] = LOGOUT_MSG;
    if (findis) {
        User user;
        user.setId(userid);
        userModel_.updateState(user, offline);
        heartMap_.erase(conn);      //为用户取消心跳检测
        redis_.unsubscribe(userid); //为用户取消订阅

        response["errno"] = 0;
        response["successmsg"] = "you logout sucess";
    } else {
        response["errno"] = -1;
        response["errmsg"] = "don't find the user!";
    }
    conn->send(response.dump());
}

void ChatService::reg(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    std::string name = js["name"].get<std::string>();
    std::string passwd = js["passwd"].get<std::string>();
    User user = userModel_.insert(name, passwd);

    json response;
    response["msgid"] = REG_ACK_MSG;

    if (user.id() == -1) {
        response["errno"] = -1;
        response["errmsg"] = "sorry, your apply is failure";
        conn->send(response.dump());
        return ;
    }

    response["errno"] = 0;
    response["newid"] = user.id();
    conn->send(response.dump());
}

void ChatService::oneChat(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = connections_.find(toid);
        if (iter != connections_.end()) {
            //说明这个人也在这个服务上连接着,直接将消息转发给他
            std::string msg = js.dump();
            iter->second->send(msg);
            return ;
        }
    }

    if (userModel_.query(toid).state() == online) {
        //如果用户在线说明它在其他服务器上登录，将消息发布到消息队列中
        redis_.publish(toid, js.dump());
    } else {
        //插入到离线消息中
        offlineMessageModel_.insert(toid, js.dump());
    }
}

void ChatService::groupChat(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int groupid = js["groupid"].get<int>();
    int userid = js["userid"].get<int>();
    std::vector<GroupUser> groupUsers = groupModel_.queryGroupUser(groupid);
    
    for (auto groupuser : groupUsers) {
        if (groupuser.id() == userid) continue; //不用给自己发送
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = connections_.find(groupuser.id());
        if (iter != connections_.end()) {
            iter->second->send(js.dump());         
        } else if (userModel_.query(groupuser.id()).state() == online) {
            //如果此时用户在线的话,直接发布到消息队列中
            redis_.publish(groupuser.id(), js.dump());
        } else {
            //用户不在线插入到离线消息中
            offlineMessageModel_.insert(groupuser.id(), js.dump());
        }
    }  
}

void ChatService::addFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int myid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    
    json response; 
    response["msgid"] = ACK_MSG;
    if (!friendModel_.addFriend(myid, friendid)) {
        //添加失败
        response["errno"] = -1;
        response["errmsg"] = "your apply failure!";   
        conn->send(response.dump());
        return ;
    }

    response["errno"] = 0;
    response["successmsg"] = "add friend success!";

    conn->send(response.dump());
}

void ChatService::deleteFriend(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int myid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    
    json response; 
    response["msgid"] = ACK_MSG;
    if (!friendModel_.deleteFriend(myid, friendid)) {
        //添加失败
        response["errno"] = -1;
        response["errmsg"] = "your apply failure!";   
        conn->send(response.dump());
        return ;
    }

    response["errno"] = 0;
    response["successmsg"] = "delete friend success!";

    conn->send(response.dump());
}

void ChatService::addGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();

    json response;
    response["msgid"] = ACK_MSG;
    if (!groupModel_.addGroup(userid, groupid)) {
        response["errno"] = -1;
        response["errmsg"] = "sorry, your apply is failure!";
        conn->send(response.dump());
        return ;
    }
    response["errno"] = 0;
    response["successmsg"] = "addgroup success!";

    conn->send(response.dump());
}


void ChatService::createGroup(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    std::string groupName = js["groupName"];
    std::string groupDesc = js["groupDesc"];
    
    Group group = groupModel_.createGroup(groupName, groupDesc);
    json response;
    response["msgid"] = ACK_MSG;
    if (group.groupId() == -1) {
        response["errno"] = -1;
        response["errmsg"] = "sorry, your apply is failure!";
        conn->send(response.dump());
        return ;
    }

    //创建成功，将创建者加入并设置为creator成员
    if (!groupModel_.addGroup(userid, group.groupId(), "creator")) {
        response["errno"] = -1;
        response["errmsg"] = "sorry, your apply is failure!";
        conn->send(response.dump());
        return ;
    }

    response["errno"] = 0;
    std::string groupid = std::to_string(group.groupId());
    response["successmsg"] = groupid + ",this is your groupid, please remember it!";

    conn->send(response.dump());
}

void ChatService::reciveHeart(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    if (heartMap_.find(conn) == heartMap_.end()) {
        MYLOG_ERROR("recive a heart message, but the client don't place int server!");
        return ;
    }
    heartMap_[conn] = 0;
}

void ChatService::clientCloseExption(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto iter = connections_.begin(); iter != connections_.end(); ++iter) {
            //由于找到这个客户删除后就不需要仅需遍历了，所以不需要考虑迭代器的失效问题
            if (iter->second == conn) {
                user.setId(iter->first);
                connections_.erase(iter->first);
                break;
            }
        }
    }

    if (user.id() != -1) {
        userModel_.updateState(user, offline);              //下线
        redis_.unsubscribe(user.id());                      //取消redis订阅
        if (heartMap_.find(conn) != heartMap_.end()) {
            heartMap_.erase(conn);
            MYLOG_INFO("a client close exption!");  
        } else {
            //如果这个用户已经删除过，说明是因为心跳包检测超时导致的
            MYLOG_INFO("a client heart test time out!"); 
        }
    }
}

//通过redis从其他服务器上转发给这个服务器上用户的消息
void ChatService::handleRedisSubscribeMessage(int userid, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = connections_.find(userid);
    if (iter != connections_.end()) {
        iter->second->send(msg);
        return ;
    }

    //用户在这段时间下线了
    offlineMessageModel_.insert(userid, msg);
}


void ChatService::heartTest()
{
    for (auto iter = heartMap_.begin(); iter != heartMap_.end(); ) {    //这样设计防止迭代器失效
        if (++iter->second == 3) {
            iter = heartMap_.erase(iter);        //取消心跳检测,同时防止迭代器失效
            clientCloseExption(iter->first);     //该客户意外的断开了
            continue;
        }
        iter++;
    }
}


void ChatService::serverCloseExption() 
{
    for (auto iter : connections_) {
        userModel_.resetState(iter.first);
        redis_.unsubscribe(iter.first);
        iter.second->shutDown();
    }
}
