#ifndef USERMODEL_H
#define USERMODEL_H

#include "deletecopy.h"
#include "server/model/user.h"

class UserModel : deletecopy
{
public:
    UserModel() = default;
    ~UserModel() = default;

    User insert(const std::string& name, const std::string& passwd);  //新建一个用户
    User query(int id);   //查询一个用户

    bool updateState(const User& user, State state);    //更新这个用户的状态信息
    bool resetState(int userid);  //重置这个用户的状态信息
private:
};

#endif 