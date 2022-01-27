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

/*
    \002\200\376\377\a\000\000\000\320\b\000\350\377\177\000\000\",\"msgid\":5,\"name\":\"zhang san\",\"time\":\"2022/01/26 23:20:00\",\"toid\":8,\"userid\":7}
    {\"message\":\"hasd\",\"msgid\":5,\"name\":\"zhang san\",\"time\":\"2022/01/26 23:20:00\",\"toid\":8,\"userid\":7}"
    101
*/
