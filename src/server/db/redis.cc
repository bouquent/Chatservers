#include "server/db/redis.h"
#include "mylog.h"
#include <thread>

Redis::Redis()
    : publishContext_(nullptr)
    , subscribeContext_(nullptr) 
{}

Redis::~Redis()
{
    if (publishContext_ != nullptr) {
        redisFree(publishContext_);
    }
    if (subscribeContext_ != nullptr) {
        redisFree(subscribeContext_);
    }
}

bool Redis::connect()
{
    publishContext_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == publishContext_) {
        MYLOG_ERROR("[%s]:%d redisConnect is error!", __FILE__, __LINE__);
        return false;
    }
    subscribeContext_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == publishContext_) {
        MYLOG_ERROR("[%s]:%d redisConnect is error!", __FILE__, __LINE__);
    }

    std::thread redisThread([&](){
        this->obsever_channel_message();
    });
    redisThread.detach();
    MYLOG_INFO("redisConnect is success!");
    return true;
}

bool Redis::subscribe(int channel)
{
    //填充命令
    if (REDIS_ERR == redisAppendCommand(subscribeContext_, "subscribe %d", channel)) {
        MYLOG_ERROR("[%s]:%d subsrcibe append command failure!", __FILE__, __LINE__);
        return false;
    }

    int done = 0;
    while (!done) {
        //将订阅消息反复发送给redis服务，直到成功订阅
        if (REDIS_ERR == redisBufferWrite(subscribeContext_, &done)) {
            MYLOG_ERROR("[%s]:%d subsrcibe buffer write failure", __FILE__, __LINE__);
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) 
{
    //填充命令
    if (REDIS_ERR == redisAppendCommand(subscribeContext_, "unsubscribe %d", channel)) {
        MYLOG_ERROR("[%s]:%d unsubsrcibe append comman failure!", __FILE__, __LINE__);
        return false;
    }

    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(subscribeContext_, &done)) {
            MYLOG_ERROR("[%s]:%d unsubsrcibe buffer write failure!", __FILE__, __LINE__);
            return false;
        }
    }
    return true;
}

bool Redis::publish(int channel, const std::string& message)
{
    redisReply* reply = (redisReply*)redisCommand(publishContext_, "publish %d %s", channel, message.c_str());
    if (nullptr == reply) {
        MYLOG_ERROR("[%s]:%d redis publish failure!", __FILE__, __LINE__);
        return false;
    }
    
    freeReplyObject(reply);
    return true;
}

void Redis::obsever_channel_message()
{
    redisReply* reply = nullptr;

    while (REDIS_OK == redisGetReply(subscribeContext_, (void**) &reply)) {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
    }

    freeReplyObject(reply);
    //如果结束循环，说明不再对通道进行监听，此时所有的服务器都应该暂停了工作
    MYLOG_INFO("observer_channel_message quit!");
}