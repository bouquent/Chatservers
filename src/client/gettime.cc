#include "client/gettime.h"
#include <time.h>
#include <stdio.h>

GetTime GetTime::now()
{
    time_t second = time(nullptr);
    return GetTime(second);
}

std::string GetTime::toString()
{
    time_t second = secondsSinceSt_;
    struct tm* time = localtime(&second);

    char totime[1024] = {0};
    snprintf(totime, 1024, "%4d/%02d/%02d %02d:%02d:%02d", 
            time->tm_year + 1900,
            time->tm_mon + 1,
            time->tm_mday,
            time->tm_hour,
            time->tm_min,
            time->tm_sec);

    return totime;
}