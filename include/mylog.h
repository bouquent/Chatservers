#ifndef MYLOG_H
#define MYLOG_H

#include <stdio.h>
#include <unistd.h>

#define MYLOG_INFO(logmsg, ...) do{ \
    MyLog log(INFO);\
    char msg[1024] = {0}; \
    snprintf(msg, 1024, logmsg, ##__VA_ARGS__);\
    log.printMsg(msg);\
} while(0);

#define MYLOG_ERROR(logmsg, ...) do{ \
    MyLog log(ERROR);\
    char msg[1024] = {0}; \
    snprintf(msg, 1024, logmsg, ##__VA_ARGS__);\
    log.printMsg(msg);\
} while(0);


//无法接受的错误，打印后直接退出
#define MYLOG_FATAL(logmsg, ...) do{ \
    MyLog log(FATAL);\
    char msg[1024] = {0}; \
    snprintf(msg, 1024, logmsg, ##__VA_ARGS__);\
    log.printMsg(msg);\
    exit(-1);\
} while(0);

enum LEVEL
{
    INFO, 
    ERROR,
    FATAL
};

class MyLog
{
public:
    MyLog(LEVEL level) : level_(level) {}

    void printMsg(const char* s) {
        switch (level_) {
        case INFO :
            printf("[C_INFO]:");
            break;
        case ERROR:
            printf("[C_ERROR]:");
            break;
        case FATAL:
            printf("[C_FATAL]:");
            break;
        default:
            break;
        }
        printf("%s\n", s);
    }

    void setLevel(LEVEL level) { level_ = level; } 

private:
    LEVEL level_;
};

#endif 