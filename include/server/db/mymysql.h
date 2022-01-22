#ifndef MYMYSQL_H
#define MYMYSQL_H

#include "deletecopy.h"
#include <mysql/mysql.h>
#include <mymuduo/logging.hpp>

class MySql : deletecopy
{
public:
    MySql() {
        conn_ = mysql_init(nullptr); 
        if (nullptr == conn_) {
            LOG_INFO("mysql_init is error!");
        }
    }
    ~MySql() {
        mysql_close(conn_);
    }
    bool connect(const char* host, const char* user, const char* passwd, const char* db, unsigned int port);
    MYSQL* getConnect() const { return conn_; }

    bool update(const char* sql);
    MYSQL_RES* query(const char* sql);

private:
    MYSQL* conn_;
};

#endif 