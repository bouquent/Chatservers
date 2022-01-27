#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "server/model/groupuser.h"
#include "server/model/group.h"
#include "deletecopy.h"

#include <vector>
#include <string>

class GroupModel : deletecopy
{
public:
    GroupModel() = default;
    ~GroupModel() = default;

    Group createGroup(const std::string& groupName, const std::string& groupDesc);
    bool addGroup(int userid, int groupid, const std::string& role = "normal");

    std::vector<Group> queryGroup(int userid);
    std::vector<GroupUser> queryGroupUser(int groupid);
private:
};

#endif