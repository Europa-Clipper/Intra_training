#include <iostream>
#include <vector>

#include "name.h"

int main(){
    std::cout << "main thread count = " << thname::count << std::endl;
    thname::count.fetch_add(1);
    std::cout << "main thread count = " << thname::count << std::endl;

    //plugins
    std::vector<std::thread>plugins;
    for(int i = 0; i < 3; ++i){
        plugins.push_back(std::thread([threadNumbers = thname::count.fetch_add(1)](){
            threadSetName("plugin-" + std::to_string(threadNumbers));

						//plugins'son1
            std::thread t1([threadNumbers, sonThred = thname::count.fetch_add(1)](){
							threadSetName("plugin" + std::to_string(threadNumbers) + "-son" + std::to_string(sonThred));
							std::cout << "sonThread created" << std::endl;
							while(1){}
            });

						//plugins'son2
						std::thread t2([threadNumbers, sonThred = thname::count.fetch_add(1)](){
							threadSetName("plugin" + std::to_string(threadNumbers) + "-son" + std::to_string(sonThred));
							std::cout << "sonThread created" << std::endl;
							while(1){}
            });
            while(1){}
            
        }));
    }

    std::cout <<"curent thread numbers is "<< plugins.size() << std::endl;
    while(1){}
    return 0;
}