#include "server/chatserver.h"
#include <mymuduo/inetaddr.hpp>
#include <mymuduo/eventloop.hpp>

int main(int argc, char** argv)
{
    if (argc !=  3) {
        printf("please vaild address and port!\n");
        exit(-1);
    }
    EventLoop loop;
    InetAddr localAddr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, localAddr, "cx");
    server.start();

    loop.loop();
    
    return 0;
}