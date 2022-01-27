#ifndef GETTIME_H
#define GETTIME_H

#include <string>

class GetTime
{
public:
    GetTime(int second = 0)
        : secondsSinceSt_(second)
    {}
    ~GetTime() = default; 

    static GetTime now();

    std::string toString();

    void setSecond(uint64_t second) { secondsSinceSt_ = second; }
    uint64_t getSecond() const { return secondsSinceSt_; } 
private:
    uint64_t secondsSinceSt_;
};

#endif 