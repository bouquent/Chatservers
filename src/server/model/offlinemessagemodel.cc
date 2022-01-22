#include "offlinemessagemodel.h"
#include "server/db/mymysql.h"

static std::string myhost = "127.0.0.1";
static std::string myuser = "root";
static std::string mypasswd = "chenzezheng666";
static std::string mydb = "chat";
static unsigned int port = 3306;

bool OfflineMessageModel::insert(int id, const std::string& message)
{
    char sql[1024] = {0}; 
    snprintf(sql, 1024, "insert into offlineMessage(userid, message) values('%d', '%s')", id, message.c_str());

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return false;
    }    
    return conn.update(sql);
}

bool OfflineMessageModel::remove(int id)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "delete from offlineMessage where userid = '%d'", id);

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return false;
    }    
    return conn.update(sql);
}

std::vector<std::string> OfflineMessageModel::query(int id)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select * from offlineMessage where userid = '%d'", id);

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return {};
    }    

    MYSQL_RES *result = conn.query(sql);
    if (result == nullptr) {
        return {};
    }

    
    std::vector<std::string> messvec;
    MYSQL_ROW row;
    while (row = mysql_fetch_row(result)) {
        messvec.push_back(std::string(row[0]));
    }
    mysql_free_result(result);

    return messvec;
}
