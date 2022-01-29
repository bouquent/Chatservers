#include <unistd.h>
#include <string.h>
#include <thread>

#include "client/clientservice.h"
#include "json.hpp"

bool client_stop = false;

int helpMenu()
{
    int opt = 0;
    std::cout << "请选择你想要的功能:" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "1. 登录" << std::endl;
    std::cout << "2. 注册" << std::endl;
    std::cout << "3. 退出" << std::endl;
    std::cout << "==================" << std::endl;

    std::cin >> opt;
    if (std::cin.fail()) { //如果输入不合法
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        return -1;
    }
    std::cin.get(); //清除缓冲区中的那个换行符号
    return opt;
}

int main(int argc,  char** argv)
{
    if (argc != 3) {
        printf("please input vaild address and port!");
        exit(1);
    }

    
    if (!ClientService::getService()->connect(argv[1], atoi(argv[2])) ) {      //建立连接
        exit(1);
    }

    while (!client_stop) {
        int opt = helpMenu();   //让用户根据帮助选择所需要的功能
        switch (opt) {
        case 1 :
        {
            int userid;
            std::string passwd;
            std::cout << "请输入你的用户id:" << std::endl;
            std::cin >> userid;
            std::cin.get();            //将上一个输入的换行符读取
            std::cout << "请输入你的用户密码:" << std::endl;
            getline(std::cin, passwd);
            if (!ClientService::getService()->loginUser(userid, passwd)) {
                //发送登录请求失败
                std::cout << "you can try it again!" << std::endl;
                break;
            }
            //thread   这里开启一个线程，专门用来接受其他用户丶群组丶服务器发送来的消息
            std::thread thread1(ClientService::readMessageTask, ClientService::getService());
            thread1.detach();

            std::thread thread2(ClientService::sendHeartMsg, ClientService::getService());
            thread2.detach();

            //主线程主要进行与用户的交接
            ClientService::getService()->mainMenu(); 
            break;
        }
        case 2 : 
        {
            std::string name;
            std::string passwd;
            std::cout << "请输入你想要的用户名和密码:" << std::endl;
            getline(std::cin, name);
            getline(std::cin, passwd);
            if (!ClientService::getService()->regNewUser(name, passwd)) {
                std::cout << "you can try it again!" << std::endl;
            }
           break;     
        }  
        case 3 :
            client_stop = true;
            break;
        default :
            std::cerr << "invaild input!" << std::endl;
            break;
        }

    }
    
    return 0;
}
