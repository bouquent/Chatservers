#ifndef OFFLINEMODEL_H
#define OFFLINEMODEL_H

#include "deletecopy.h"
#include <string>
#include <vector>

class OfflineMessageModel : deletecopy
{
public:
    OfflineMessageModel() = default;
    ~OfflineMessageModel() = default;

    //查找userid的离线消息
    std::vector<std::string> query(int id);
    //给userid插入离线消息
    bool insert(int id, const std::string& message);
    //移除离线消息
    bool remove(int id);
private:
};

#endif 