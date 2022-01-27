#ifndef PUBLIC_H
#define PUBLIC_H

enum MSG_ID
{
    LOGIN_MSG = 0,          //登录消息类型
    LOGIN_ACK_MSG,          //响应登录类型
    LOGOUT_MSG,             //退出登录类型
    REG_MSG,                //注册消息类型
    REG_ACK_MSG,            //响应注册类型

    ONE_CHAT_MSG,            //点对点聊天
    GROUP_CHAT_MSG,         //群聊

    ADD_FRIEND_MSG,          //添加好友
    DELETE_FRIEND_MSG,       //删除好友
    ADD_GROUP,               //添加群组
    CREATE_GROUP,            //创建群组

    ACK_MSG,                //回应客户

    HEART_MSG,              //心跳包消息
};

#endif 
