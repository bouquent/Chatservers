#ifndef USERMODEL_H
#define USERMODEL_H

#include "server/db/mymysql.h"
#include "deletecopy.h"
#include "server/model/user.h"

class UserModel : deletecopy
{
public:
    UserModel() = default;
    ~UserModel() = default;

    bool insert(User& user);  //插入一个用户
    User query(int id);   //查询一个用户

    bool updateState(const User& user, State state);    //更新这个用户的状态信息
    bool resetState(const User& user);  //重置这个用户的状态信息
private:
};

#endif 