#include<iostream>
#include"03vector.h"

using namespace qyk;

int main() {
	vector<int> a;
	for (int i = 0; i != a.size(); ++i) {
		std::cout << i << "  "<<a[i]<< std::endl;
	}
	std::cout <<"a" << std::endl;

	vector<int> b(5, 2);
	for (int i = 0; i != b.size(); ++i) {
		std::cout << i << "  " << b[i] << std::endl;
	}std::cout << std::endl;

	vector<int> c(5);
	for (int i = 0; i != c.size(); ++i) {
		std::cout << i << "  " << c[i] << std::endl;
	}std::cout << std::endl;
	
	vector<int> d(b.begin(),b.end());
	for (int i = 0; i != d.size(); ++i) {
		std::cout << i << "  " << d[i] << std::endl;
	}std::cout << std::endl;

	vector<int> e(d);
	for (int i = 0; i != e.size(); ++i) {
		std::cout << i << "  " << e[i] << std::endl;
	}std::cout << std::endl;

	vector<int> f(std::move(e));
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	std::cout << "e.size（）" << e.size();


	int* p = f.data();
	std::cout << p << "  " << f.begin();

	d.clear();
	for (int i = 0; i != d.size(); ++i) {
		std::cout << i << "  " << d[i] << std::endl;
	}std::cout << std::endl;

	f.resize(20);
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.insert(f.begin()+2, 10);
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.insert(f.begin() + 2,2,9);
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.insert(f.begin() + 2, b.begin(),b.end());
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.insert(f.begin() + 2, f.begin(), f.end());
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.push_back(66);
	f.emplace_back(66);
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	f.erase(f.begin() + 5, f.begin() + 10);
	for (int i = 0; i != f.size(); ++i) {
		std::cout << i << "  " << f[i] << std::endl;
	}std::cout << std::endl;

	return 0;

}
