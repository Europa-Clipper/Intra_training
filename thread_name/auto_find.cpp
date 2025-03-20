#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <vector>
#include <dirent.h>

#include "/home/nakanomiku/wxx/intra-train/include/cmdline-master/cmdline.h"


bool checkProcessName(int pid, const std::string targetname){
        std::string path = "/proc/" + std::to_string(pid) + "/comm";
        std::ifstream file(path);
        if(file.is_open()){
            std::string procName;
            std::getline(file, procName);
            file.close();

            return procName == targetname;
        }
}


std::vector<int> findProcByName(const std::string& targetname){
    std::vector<int>procs;

    DIR* dir = opendir("/proc");
    if(dir != nullptr){
        struct dirent* entry;
        while((entry = readdir(dir)) != nullptr){
            if(entry -> d_type == DT_DIR){
                std::string dirName = entry -> d_name;
                try{
                    int pid = std::stoi(dirName);
                    if(checkProcessName(pid, targetname)) {
                        procs.push_back(pid);
                        std::cout<< "found pid : " << pid <<std::endl;
                    }
                } catch(const std::invalid_argument&){
                    continue;
                }
            }
        }
        closedir(dir);
    }
    return procs;
}

int main(int argc, char *argv[]){
    cmdline::parser cl;
    cl.add<std::string>("name", 'n', "process name", true, "");
    cl.add<std::string>("task", 't', "task todo", true, "find");+
    //cl.add<int>("pid", 'p', "process pid", false, 0);
    
    cl.parse_check(argc, argv);

    std::cout << "process name:" << cl.get<std::string>("name") << std::endl
       //<< cl.get<std::string>("host") << ":"
    << "TASK = " <<cl.get<std::string>("task") << std::endl;

    findProcByName(cl.get<std::string>("name"));
    return 0;
}


