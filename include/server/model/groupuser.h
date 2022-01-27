#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "server/model/user.h"
#include <string>



class GroupUser : public User
{
public:
    GroupUser(int id = -1, const std::string& name = "", const std::string& passwd = "",
                State state = offline, const std::string& grouprole = "normal") 
    {}
    ~GroupUser() = default;

    std::string groupRole() const { return groupRole_; }
    void setGroupRole(const std::string& groupRole) { groupRole_ = groupRole; }
private:
    std::string groupRole_;  //在群中的身份
};

#endif