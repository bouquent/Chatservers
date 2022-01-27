#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "deletecopy.h"
#include "user.h"
#include <vector>

class FriendModel : deletecopy
{
public:
    FriendModel() = default;
    ~FriendModel() = default;

    bool addFriend(int myid, int friendid);
    bool deleteFriend(int myid, int friendid);
    std::vector<User> queryFriend(int myid);
private:
};



#endif 
