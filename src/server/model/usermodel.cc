#include "server/model/usermodel.h"
#include "server/db/mymysql.h"

#include <string.h>

static std::string myhost = "127.0.0.1";
static std::string myuser = "root";
static std::string mypasswd = "chenzezheng666";
static std::string mydb = "chat";
static unsigned int port = 3306;

bool UserModel::insert(User& user)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into User(name, password, state) values('%s', '%s', 'offline')",  //刚新建的时候一定是未登录的
                    user.name().c_str(), user.passwd().c_str());

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return false;
    }
    
    if (conn.update(sql) == true) {
        user.setId(mysql_insert_id(conn.getConnect()));
        return true;
    }
    return false;
}

User UserModel::query(int id) 
{
    char sql[1024] = {0};
    snprintf(sql, 1204, "select * from User where id = %d", id);

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return User();
    }
    
    MYSQL_RES* result = conn.query(sql);
    if (nullptr == result) {
        return User();
    }
    MYSQL_ROW row = mysql_fetch_row(result);

    User user;
    user.setId(id);
    user.setName(row[1]);
    user.setPasswd(row[2]);
    if (strcmp(row[3], "offline") == 0) {
        user.setState(offline);
    } else {
        user.setState(online);
    }

    mysql_free_result(result);
    return user;
} 

bool UserModel::updateState(const User& user, State updatestate)
{
    std::string state;
    if (updatestate == online) {
        state = "online";
    } else {
        state = "offline";
    }
    int id = user.id();
    
    char sql[1024] = {0}; 
    snprintf(sql, 1024, "update User set state = '%s' where id = %d", state.c_str(), id);

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return false;
    }
    if (conn.update(sql) == false) {
        return false;
    }
    return true;
    
}

bool UserModel::resetState(const User& user)
{
    int id = user.id();
    char sql[1024] = {0}; 
    snprintf(sql, 1024, "update User set state = 'offline' where id = %d", id);

    MySql conn;
    if (conn.connect(myhost.c_str(), myuser.c_str(), mypasswd.c_str(), mydb.c_str(), port) == false) {
        return false;
    }
    if (conn.update(sql) == false) {
        return false;
    }
    return true;

}