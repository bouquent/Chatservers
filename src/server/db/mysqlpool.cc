#include "server/db/mysqlpool.h"
#include "mylog.h"
#include <unistd.h>
#include <strings.h>
#include <thread>

MySqlPool* MySqlPool::getMysqlPool()
{
    static MySqlPool mysqlPool;
    return &mysqlPool;
}

MySqlPool::MySqlPool() 
{
    loadMysqlConfig();

    for (int i = 0; i < minConnectNum_; ++i) {
        MySql* conn = new MySql;
        if (!conn->connect(host_.c_str(), user_.c_str(), passwd_.c_str(), db_.c_str(), port_)) {
            MYLOG_ERROR("[%s]:%d mysql connect error\n", __FILE__, __LINE__);
        }
        conn->refreshAliveTime();
        connectQue_.push(conn);
        connectNum_++;
    }

    std::thread thread1(MySqlPool::scannerOverFreeTime, this);
    thread1.detach();

    std::thread thread2(MySqlPool::produceMysqlConnect, this);
    thread2.detach();
}

MySqlPool::~MySqlPool()
{
    for (int i = 0; i < connectNum_; ++i) {
        MySql* conn = connectQue_.front();
        connectQue_.pop();
        delete conn;
    }
}


std::shared_ptr<MySql> MySqlPool::getConnect()
{   
    std::unique_lock<std::mutex> lock(mutex_);
    while (connectQue_.empty()) {
        if (cond_.wait_for(lock, std::chrono::milliseconds(connectOutTime_)) == std::cv_status::timeout) {
            if (connectQue_.empty()) {
                MYLOG_ERROR("[%s]:%d 连接失败，获取连接超时\n", __FILE__, __LINE__);
                return nullptr;
            }
        }
    }

    std::shared_ptr<MySql> sp(connectQue_.front(), [&](MySql* conn){
            std::lock_guard<std::mutex> lock(mutex_);
            connectQue_.push(conn);
            conn->refreshAliveTime();
    });

    connectQue_.pop();
    cond_.notify_all(); //唤醒生产者，如果队列为空生产者开始生成更多的connection
    return sp;
    
}

void MySqlPool::scannerOverFreeTime(MySqlPool* mysqlPool)
{
    while (1) {
        sleep(mysqlPool->maxFreeTime_);   //模拟定时效果

        while (mysqlPool->connectNum_ > mysqlPool->minConnectNum_) {
            std::lock_guard<std::mutex> lock(mysqlPool->mutex_);
            MySql* conn = mysqlPool->connectQue_.front();
            if (mysqlPool->maxFreeTime_ * CLOCKS_PER_SEC < conn->getAliveTime()) {
                //这个connection的空闲时间过长直接删除
                mysqlPool->connectQue_.pop();
                delete conn;
                mysqlPool->connectNum_--;
            } else {
                //如果队头的定时器都没有超过，后面的肯定也不会超过
                break;
            }
        }
    }
}

void MySqlPool::produceMysqlConnect(MySqlPool* mysqlPool)
{
    while (1) {
        std::unique_lock<std::mutex> lock(mysqlPool->mutex_);

        while (!mysqlPool->connectQue_.empty()) {
            //如果connection句柄不为空，那么不需要生产更多的connection，让生产者阻塞在这里
            mysqlPool->cond_.wait(lock);    
        }

        if (mysqlPool->maxConnectNum_ > mysqlPool->connectNum_) {
            MySql* conn = new MySql();
            conn->connect(mysqlPool->host_.c_str(), mysqlPool->user_.c_str(), 
                        mysqlPool->passwd_.c_str(), mysqlPool->db_.c_str(), mysqlPool->port_);
            conn->refreshAliveTime();
            mysqlPool->connectQue_.push(conn);
        }

        mysqlPool->cond_.notify_all();  //唤醒消费者线程
    }
}

void MySqlPool::trimString(std::string& s)    //去除字符串两边的空格以及换行符
{
    int left, right = 0;
    for (left = 0; left < s.size(); ++left) {
        if (s[left] != ' ' && s[left] != '\n') break;
    }
    for (right = s.size() - 1; right >= 0; --right) {
        if (s[right] != ' ' && s[right] != '\n') break;
    }
    if (left > right) {
        std::string().swap(s);
        return ;
    }

    s = s.substr(left, right - left + 1);
}


void MySqlPool::loadMysqlConfig()
{
    FILE* file = fopen("mysqlconfig.txt", "r");
    if (nullptr == file) {
        MYLOG_ERROR("[%s]:%d fopen wrong, load mysqlconfig failure!\n", __FILE__, __LINE__);
        return ;
    }

    char buf[1024] = {0};
    while (fgets(buf, sizeof(buf), file)) {
        std::string s = buf;
        trimString(s);
        int idx = s.find(":");
        std::string key = s.substr(0, idx);
        std::string value = s.substr(idx + 1, s.size() - idx - 1);

        if (strncasecmp(key.c_str(), "host", 4) == 0) {
            host_ = value;
        } else if (strncasecmp(key.c_str(), "user", 4) == 0) {
            user_ = value;
        } else if (strncasecmp(key.c_str(), "db", 2) == 0) {
            db_ = value;
        } else if (strncasecmp(key.c_str(), "passwd", 6) == 0) {
            passwd_ = value;
        } else if (strncasecmp(key.c_str(), "port", 4) == 0) {
            port_ = atoi(value.c_str());
        } else if (strncasecmp(key.c_str(), "ConnectOutTime", 14) == 0) {
            connectOutTime_ =  atoi(value.c_str());
        } else if (strncasecmp(key.c_str(), "maxFreeTime", 11) == 0) {
            maxFreeTime_ = atoi(value.c_str());
        } else if (strncasecmp(key.c_str(), "mincountNum", 11) == 0) {
            minConnectNum_ = atoi(value.c_str());
        } else if (strncasecmp(key.c_str(), "maxcountNum", 11) == 0) {
            maxConnectNum_ = atoi(value.c_str());
        }
    }
    connectNum_ = 0;
}
