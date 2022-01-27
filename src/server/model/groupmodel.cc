#include "server/model/groupmodel.h"
#include "server/db/mysqlpool.h"


Group GroupModel::createGroup(const std::string& groupName, const std::string& groupDesc)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
                        groupName.c_str(), groupDesc.c_str());

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();
   

    if (conn->update(sql) == true) {
        Group group;
        group.setGroupDesc(groupDesc);
        group.setGroupName(groupName);
        group.setGroupId(mysql_insert_id(conn->getConnect()));
        return group;
    }
    
    return Group();
}

bool GroupModel::addGroup(int userid, int groupid, const std::string& role) 
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')", 
                        userid, groupid, role.c_str());

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();
    
    return conn->update(sql);
}

std::vector<GroupUser> GroupModel::queryGroupUser(int groupid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select id, name, state, grouprole from User right join GroupUser \
                        on User.id = GroupUser.userid where groupid = %d", groupid);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    MYSQL_RES *result = conn->query(sql);
    if (nullptr == result) {
        return {};
    }

    std::vector<GroupUser> groupUsers;
    MYSQL_ROW row;
    while (row = mysql_fetch_row(result)) {
        GroupUser groupuser;
        groupuser.setId(atoi(row[0]));
        groupuser.setName(row[1]);
        groupuser.setGroupRole(row[3]);
        std::string state = row[2];
        if (state == "online") groupuser.setState(online);
        else groupuser.setState(offline);

        groupUsers.push_back(groupuser);
    }
    return groupUsers;

    mysql_free_result(result);
    return groupUsers;
}



std::vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select groupid, groupname, groupdesc from AllGroup where groupid In \
                (select groupid from GroupUser where userid = %d)", userid);

    std::shared_ptr<MySql> conn = MySqlPool::getMysqlPool()->getConnect();

    MYSQL_RES *result = conn->query(sql);
    if (nullptr == result) {
        return {};
    }

    MYSQL_ROW row;
    std::vector<Group> groups;
    while (row = mysql_fetch_row(result)) {
        Group grouptmp;
        grouptmp.setGroupId(atoi(row[0]));
        grouptmp.setGroupName(row[1]);
        grouptmp.setGroupDesc(row[2]);
        groups.push_back(grouptmp);
    }
    mysql_free_result(result);

    return groups;
}
