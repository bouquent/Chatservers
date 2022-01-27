#ifndef REDIS_H
#define REDIS_H

#include <string>
#include <functional>
#include <hiredis/hiredis.h>

class Redis
{
public:
    Redis();
    ~Redis();
    bool connect();

    //订阅通道
    bool subscribe(int channel);
    bool unsubscribe(int channel);

    //向channel通道中发布消息
    bool publish(int channel, const std::string& message);

    //观察通道中的消息，如果有消息通过服务器注册的notify_message_handler回调函数，通知相应的服务器
    void obsever_channel_message();

    void setNotify_message_handler(std::function<void(int, std::string)> cb) { notify_message_handler = cb; }
    
private:
    redisContext* publishContext_;
    redisContext* subscribeContext_;
    
    std::function<void(int, std::string)> notify_message_handler; 
};


#endif