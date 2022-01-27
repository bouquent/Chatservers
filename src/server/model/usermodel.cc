#include "server/model/usermodel.h"
#include "server/db/mysqlpool.h"

#include <string.h>

User UserModel::insert(const std::string& name, const std::string& passwd)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into User(name, password, state) values('%s', '%s', 'offline')",  //刚新建的时候一定是未登录的
                    name.c_str(), passwd.c_str());

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    if (conn->update(sql) == true) {
        User user;
        user.setName(name);
        user.setPasswd(passwd);
        user.setId(mysql_insert_id(conn->getConnect()));
        return user;
    }
    return User();
}

User UserModel::query(int id) 
{
    char sql[1024] = {0};
    snprintf(sql, 1204, "select id, name, password, state from User where id = %d", id);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();
    
    MYSQL_RES* result = conn->query(sql);
    if (nullptr == result) {
        return User();
    }
    MYSQL_ROW row = mysql_fetch_row(result);

    User user;
    user.setId(atoi(row[0]));
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

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    return conn->update(sql);
}

bool UserModel::resetState(int userid)
{
    char sql[1024] = {0}; 
    snprintf(sql, 1024, "update User set state = 'offline' where id = %d", userid);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    return conn->update(sql);
}