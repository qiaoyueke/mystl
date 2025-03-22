#include"../src/01pool_allocator.h"
#include"../src/01qyk_allocator.h"
#include<iostream>

using namespace qyk;

int main() {
	//int* p = allocator<int>::allocate();
	//std::cout << p << std::endl;
	for (int i = 0; i < 100; i++) {
		int* p= allocator<int>::allocate();
		std::cout <<i+1<<"  "<< p << std::endl;
	}
	return 0;
}
