#include"../src/12map.h"
#include"../src/03vector.h"
#include"../src/13multimap.h"
using namespace qyk;

int main(){
    
    map<int,int> a;
    vector<std::pair<int,int>> b{{9,9},{8,8},{7,7},{6,6},{5,5},{4,4},{3,3},{2,2},{1,1},{0,0}};
    map<int,int> c(b.begin(),b.end());
    c[5]=10;
    for(auto i=c.begin(); i!=c.end(); i++){
        std::cout<<(*i).second<<"    ";
    } 
    std::cout<<(*c.lower_bound(9)).second;
    std::cout<<(*c.upper_bound(4)).second;
    std::cout<<std::endl;
    
    multimap<int,int> a1;
    vector<std::pair<int,int>> b1{{9,9},{9,9},{8,8},{7,7},{6,6},{5,5},{4,4},{3,3},{2,2},{1,1},{0,0}};
    multimap<int,int> c1(b1.begin(),b1.end());
    c1[9]=10;
    for(auto i=c1.begin(); i!=c1.end(); i++){
        std::cout<<(*i).first<<"    ";
    } 
    std::cout<<(*c.lower_bound(9)).second;
    std::cout<<(*c.upper_bound(5)).second;
    
}
