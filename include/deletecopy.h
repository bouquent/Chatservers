#ifndef DELETECOPY_H
#define DELETECOPY_H

class deletecopy
{
public:
    deletecopy(const deletecopy&) = delete;
    deletecopy& operator=(const deletecopy&) = delete;

protected:
    deletecopy() = default;
    ~deletecopy() = default;
};

#endif 
