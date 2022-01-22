#ifndef PUBLIC_H
#define PUBLIC_H

enum MSG_ID
{
    LOGIN_MSG = 0,          //登录消息类型
    LOGIN_ACK_MSG,          //响应登录类型
    LOGOUT_MSG,             //退出登录类型
    REG_MSG,                //注册消息类型
    REG_ACK_MSG,            //响应注册类型

    ONECHAT_MSG,            //点对点聊天
};

#endif 
