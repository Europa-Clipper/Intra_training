#include <iostream>
#include <random>

int main(){
  int yygx{0}, llss{0};

  std::random_device rd; 
  std::mt19937 gen(rd()); // Mersenne Twister 引擎
  std::uniform_int_distribution<> dis(2, 1002); 

  for(int i = 0; i < 10000; ++i){
    int random_number = dis(gen);
    std::cout << "Random number: " << random_number << std::endl;
    if(random_number % 2 == 0){++llss;}
    else{++yygx;}
  }
  std::cout<<"go or not:"<<(yygx >= llss ? "冲！" : "老实")<<std::endl<<"yygx:"<<yygx<<"  llss:"<<llss<<std::endl;
  // int random_number = dis(gen);
  // std::cout << "Random number: " << random_number << std::endl;
  return 0;
}