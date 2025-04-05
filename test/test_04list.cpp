#include"../src/04list.h"
#include"../src/03vector.h"

using namespace qyk;

int main(){
    list<int> a(2,2);
    list<int> b(3,3);
    for(auto i =b.begin();i!=b.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<b.size()<<"    遍历b"<<std::endl;
    
    b.push_front(1);
    
    for(auto i =b.begin();i!=b.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<b.size()<<"    b.push_front(1)后遍历b"<<std::endl;

    b.insert(b.begin(),6,6);
    for(auto i =b.begin();i!=b.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<b.size()<<"    b.insert(b.begin(),6,6)后遍历;"<<std::endl;


    b.insert(b.begin(),a.begin(),a.end());

    for(auto i =b.begin();i!=b.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<b.size()<<"    b.insert(b.begin(),a.begin(),a.end());后遍历;"<<std::endl;

    list<int> c(std::move(b));
    for(auto i =c.begin();i!=c.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<c.size()<<"    list<int> c(std::move(b));后遍历c;"<<std::endl;

    for(auto i =b.begin();i!=b.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<b.size()<<"    list<int> c(std::move(b));后遍历b;"<<std::endl;

    c.remove(6);

    c.erase(c.begin());

    for(auto i =c.begin();i!=c.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<c.size()<<"    c.remove(6);c.erase(c.begin());后遍历c;"<<std::endl;


    
    std::cout<<c.front()<<"  "<<c.back()<<"    c.front(),c.back();"<<std::endl;
    c.reverse();

    for(auto i =c.begin();i!=c.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<c.size()<<"    c.reverse();后遍历c;"<<std::endl;

    c.sort();
    for(auto i =c.begin();i!=c.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<c.size()<<"    c.sort();后遍历c;"<<std::endl;

    
    std::cout<<c.front()<<"  "<<c.back()<<"    c.front(),c.back();"<<std::endl;

    c.erase(c.begin(),c.end());
    for(auto i =c.begin();i!=c.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<c.size()<<"    c.erase(c.begin(),c.end());"<<std::endl;

    qyk_vector<double> va(5,1.2);
    for(auto i=va.begin();i!=va.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<va.size()<<"   vector构造"<<std::endl;

    list<double> d(a.begin(),a.end());
    for(auto i=d.begin();i!=d.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<d.size()<<"   从a构造d"<<std::endl;
    
    list<double> e(va.begin(),va.end());
    for(auto i =e.begin();i!=e.end();++i){
        std::cout<<*i<<"    ";
    }std::cout<<e.size()<<"    从vector构造"<<std::endl;
    return 0;

}