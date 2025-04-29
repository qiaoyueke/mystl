#include"../src/14hash_table.h"
#include<iostream>
using namespace qyk;

int main(){
    unsigned int a = detail::__stl_next_prime(100);
    std::cout<<a;

    detail::hash_table<int,std::pair<int,int>,select1st<int,int>,Equal<int>,std::hash<int>> b(10,std::hash<int>(),Equal<int>());
    return 0;
}