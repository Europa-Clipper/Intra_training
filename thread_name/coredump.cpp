#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <pthread.h>

int main(){
    std::vector<std::thread>vec;

    for(int i = 0 ;i < 8; ++i){
        vec.push_back(std::thread([i](){
            std::string tmp = "my_thread_" + std::to_string(i);
            pthread_setname_np(pthread_self(), tmp.c_str());
            int num = -100;
            while(true){
                ++num;
                std::cout << tmp << " = " << num << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            
        }));
    }

    vec[0].join(); //不join的话主线程创建完子线程之后进程会退出，然后会导致资源回收，子线程coredump
    std::cout<<"run success"<<std::endl;
    return 0;
}