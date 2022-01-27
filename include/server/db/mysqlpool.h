#ifndef MYSQLPOOL_H
#define MYSQLPOOL_H

#include "deletecopy.h"
#include "mymysql.h"
#include <queue>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>

class MySqlPool : deletecopy
{
public:
    static MySqlPool* getMysqlPool();
    ~MySqlPool();

    std::shared_ptr<MySql> getConnect();   //获取MySql*

private:
    MySqlPool();

    void trimString(std::string& s);    //清除字符串两边的空格和换行符
    void loadMysqlConfig();             //加载数据库信息

    static void scannerOverFreeTime(MySqlPool*);  //扫描清除空闲时间过长的connection的线程
    static void produceMysqlConnect(MySqlPool*);  //专门生产connection的线程

private:
    //连接信息
    std::string host_;
    std::string user_;
    std::string passwd_;
    std::string db_;
    int port_;


    int maxConnectNum_;     //连接数限制
    int minConnectNum_;
    int connectNum_;          //当前连接数
    int maxFreeTime_;       //最大空闲时间
    int connectOutTime_;    //连接超时时间

    std::queue<MySql*> connectQue_; 
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif 