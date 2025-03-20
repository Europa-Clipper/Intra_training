//#include "name.h"
#include <iostream>
#include <atomic>


// namespace thname {
//   thread_local std::atomic_int count{0};
// }

bool threadSetName(std::string name) {
  if(name.length() > 16 || name.length() == 0){
    std::cout << "name is invalid" << std::endl;
    return false;
  }

  if(pthread_setname_np(pthread_self(), name.c_str()) != 0) 

  return true;
}