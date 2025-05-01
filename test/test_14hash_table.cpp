#include"../src/14hash_table.h"
#include<iostream>
using namespace qyk;

int main(){
    unsigned int a = detail::__stl_next_prime(100);
    std::cout<<a<<std::endl;

    detail::hash_table<int,std::pair<int,int>,select1st<int,int>,Equal<int>,std::hash<int>> b(10,std::hash<int>(),Equal<int>());

    b.begin();
    b.end();
    b.empty();
    for(int i = 0; i < 12; i++)
    b.insert_equal({1,1});
    b.erase(b.begin());
    for(auto i = b.begin(); i != b.end(); i++){
        std::cout<<(*i).second<<"  ";
    }
    return 0;
}