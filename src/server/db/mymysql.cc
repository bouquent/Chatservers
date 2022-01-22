#include "server/db/mymysql.h"

bool MySql::connect(const char* host, const char* user, const char* passwd, const char* db, unsigned int port)
{
    MYSQL* p = mysql_real_connect(conn_, host, user, passwd, db, port, nullptr, 0);
    if (nullptr == p) {
        LOG_ERROR("[%s]:%d mysql_real_connect error!", __FILE__, __LINE__);
        return false;
    }  else {
        //防止出现中文乱码问题
        mysql_query(conn_, "set names gbk"); 

    }
    return true;
}

bool MySql::update(const char* sql)
{
    int ret = mysql_query(conn_, sql);
    if (0 != ret) {
        LOG_ERROR("[%s]:%d mysql_query is error!, error is [%s]", __FILE__,  __LINE__, mysql_error(conn_));
        return false;
    }
    return true;
}

MYSQL_RES* MySql::query(const char* sql)
{
    int ret = mysql_query(conn_, sql);
    if (0 != ret) {
        LOG_ERROR("[%s]:%d mysql_query is error!, error is [%s]", __FILE__, __LINE__, mysql_error(conn_));
        return nullptr;
    }
    return mysql_store_result(conn_);
}