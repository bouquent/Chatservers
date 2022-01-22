#ifndef USER_H
#define USER_H


#include <string>

enum State
{
    offline = 0,
    online = 1,
};

class User 
{
public:
    User(int id = -1, const std::string& name = "", const std::string& passwd = "", State state = offline)
        : id_(id)
        , name_(name)
        , passwd_(passwd)
        , state_() 
    {}
    ~User() = default;
    
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setPasswd(const std::string& passwd) { passwd_ = passwd; } 
    void setState(State state) { state_ = state; }

    int id() const { return id_; }
    std::string name() const { return name_; }
    std::string passwd() const { return passwd_; }
    State state() const { return state_; }

private:
    int id_;
    std::string name_;
    std::string passwd_;
    State state_;
};

#endif 