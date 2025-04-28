#include"../src/14hash_table.h"
#include<iostream>
using namespace qyk;

int main(){
    unsigned int a = detail::__stl_next_prime(100);
    std::cout<<a;
    return 0;
}