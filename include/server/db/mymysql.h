#ifndef MYMYSQL_H
#define MYMYSQL_H

#include "deletecopy.h"
#include <time.h>
#include <mysql/mysql.h>
#include <stdio.h>

class MySql : deletecopy
{
public:
    MySql();
    ~MySql();
    
    bool connect(const char* host, const char* user, const char* passwd, const char* db, unsigned int port);
    MYSQL* getConnect() const { return conn_; }

    bool update(const char* sql);
    MYSQL_RES* query(const char* sql);

    clock_t getAliveTime() const { return clock() - aliveTime_; }
    void refreshAliveTime() { aliveTime_ = clock(); } 
private:
    MYSQL* conn_;

    clock_t aliveTime_; //空闲时间
};

#endif 