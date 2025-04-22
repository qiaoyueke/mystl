#include"../src/10set.h"
#include"../src/03vector.h"
#include"../src/11multiset.h"
using namespace qyk;

int main(){
    set<int> a;
    vector<int> b{9,8,7,6,5,4,3,2,1,0};
    set<int> c(b.begin(),b.end());
    for(auto i=c.begin(); i!=c.end(); i++){
        std::cout<<*i<<"    ";
    }
    std::cout<<*c.lower_bound(9);

    multiset<int> a1;
    vector<int> b1{9,8,7,6,5,4,3,2,1,0};
    multiset<int> c1(b1.begin(),b1.end());
    for(auto i=c1.begin(); i!=c1.end(); i++){
        std::cout<<*i<<"    ";
    }
    std::cout<<*c1.lower_bound(9);
}
