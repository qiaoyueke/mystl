#include"../src/01loki_allocator.h"
#include"../src/01qyk_allocator.h"
#include<iostream>

using namespace qyk;

int main() {
	//int* p = allocator<int, loki_alloc>::allocate();
	//std::cout << p << std::endl;
    int* p[10000];
	for (int i = 0; i < 130; i++) {
		p[i]= allocator<int, loki_alloc>::allocate();
		std::cout << p[i] << std::endl;
	
    }
    for (int i = 0; i < 130; i++){
        std::cout<<i<<"  "<<std::endl;
	    allocator<int, loki_alloc>::deallocate(p[i]);
		//std::cout << p << std::endl;
    }

	return 0;
}

