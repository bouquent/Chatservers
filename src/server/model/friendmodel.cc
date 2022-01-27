#include "server/model/friendmodel.h"
#include "server/db/mysqlpool.h"

bool FriendModel::addFriend(int myid, int friendid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into Friend(userid, friendid) values(%d, %d)", myid, friendid);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    return conn->update(sql);
}

bool FriendModel::deleteFriend(int myid, int friendid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "delete from Friend where userid = %d and friendid = %d", myid, friendid);
    
    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    return conn->update(sql);
}

std::vector<User> FriendModel::queryFriend(int myid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select id, name, password, state from User where id In \
                             (select friendid from Friend where userid = %d)", myid);
    
    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();
    

    MYSQL_RES* result = conn->query(sql);
    if (result == nullptr) {
        return {};
    }

    std::vector<User> friends;
    MYSQL_ROW row;
    while (row = mysql_fetch_row(result)) {
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPasswd(row[2]);
        std::string state = row[3];
        if (state == "online") user.setState(online);
        else user.setState(offline);
        friends.push_back(user);
    }    
    mysql_free_result(result);
    return friends;
}
