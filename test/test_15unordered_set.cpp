#include"../src/15unordered_set.h"
#include"../src/03vector.h"
#include"../src/16unordered_multiset.h"
//#include"../src/11multiset.h"
using namespace qyk;

int main(){
    unordered_set<int> a;
    vector<int> b{9,8,7,6,5,4,3,2,1,0};
    unordered_set<int> c(b.begin(),b.end());
    for(auto i=c.begin(); i!=c.end(); i++){
        std::cout<<*i<<"    ";
    }
    std::cout<<c.size()<<"    "<<std::endl;
    a.swap(c);

    a.erase(a.begin());
    a.erase(9);
    a.resize(100);

    for(auto i=a.begin(); i!=a.end(); i++){
        std::cout<<*i<<"    ";
    }
    std::cout<<c.empty()<<a.empty()<<a.size()<<"    ";

    unordered_multiset<int> a1;
    vector<int> b1{9,8,7,6,6,6,5,4,3,2,1,0};
    unordered_multiset<int> c1(b1.begin(),b1.end());
    for(auto i=c1.begin(); i!=c1.end(); i++){
        std::cout<<*i<<"    ";
    }
    std::cout<<c1.empty()<<c1.size()<<"    ";
}
