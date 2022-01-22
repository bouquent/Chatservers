#include "server/chatserver.h"
#include <mymuduo/eventloop.hpp>
#include <mymuduo/inetaddr.hpp>
#include <mymuduo/logging.hpp>

int main()
{
    EventLoop loop;
    InetAddr localAddr("0.0.0.0", 907);
    ChatServer server(&loop, localAddr, "cx");
    server.start();

    loop.loop();
    
    LOG_INFO("server is stop!");
    return 0;
}