#include"../src/17unordered_map.h"
#include"../src/03vector.h"
#include"../src/18unordered_multimap.h"
using namespace qyk;

int main(){
    
    unordered_map<int,int> a;
    vector<std::pair<int,int>> b{{9,9},{8,8},{7,7},{6,6},{5,5},{4,4},{3,3},{2,2},{1,1},{0,0}};
    unordered_map<int,int> c(b.begin(),b.end());
    c[5]=10;
    for(auto i=c.begin(); i!=c.end(); i++){
        std::cout<<(*i).second<<"    ";
    } 
    std::cout<<c.size();
    std::cout<<std::endl;
    
    unordered_multimap<int,int> a1;
    vector<std::pair<int,int>> b1{{9,9},{9,9},{8,8},{7,7},{6,6},{5,5},{4,4},{3,3},{2,2},{1,1},{0,0}};
    unordered_multimap<int,int> c1(b1.begin(),b1.end());
    c1[9]=10;
    for(auto i=c1.begin(); i!=c1.end(); i++){
        std::cout<<(*i).first<<"    ";
    } 
    
    std::cout<<c1.size();
    
}
