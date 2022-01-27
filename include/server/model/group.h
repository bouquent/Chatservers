#ifndef GROUP_H
#define GROUP_H

#include <string>

class Group
{
public:
    Group(int groupid = -1, const std::string& groupName = "", const std::string& groupDesc = "")
            : groupid_(groupid)
            , groupName_(groupName)
            , groupDesc_(groupDesc) 
    {}
    ~Group() = default;

    void setGroupId(int groupid) { groupid_ = groupid; }
    void setGroupName(const std::string& groupName) { groupName_ = groupName; }
    void setGroupDesc(const std::string& groupDesc) { groupDesc_ = groupDesc; }
 
    int groupId() const { return groupid_; }
    std::string groupName() const { return groupName_; }
    std::string groupDesc() const { return groupDesc_; }
private:
    int groupid_;
    std::string groupName_;
    std::string groupDesc_;
};

#endif