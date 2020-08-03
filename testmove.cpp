#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <utility>
#include <chrono>
#include <vector>


class CMove {
public:
	CMove (void) {
	}

	explicit CMove (const char *p) {
		m = p;
	}

	virtual ~CMove (void) {
	}

	// ---------- copy ----------
	CMove (CMove &obj) {
		m = obj.m;
	}

	CMove& operator= (CMove &obj) {
		m = obj.m;
		return *this;
	}

	// ---------- move ----------
	CMove (CMove &&obj) noexcept {
		m = std::move(obj.m);
	}

	CMove& operator= (CMove &&obj) {
		m = std::move(obj.m);
		return *this;
	}

private:

	std::string m ;

};

void f(void)
{
	{
		std::cout << "--copy--" << std::endl;
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		std::vector<CMove> v;
		for (int i = 0; i < 1000; ++ i) {
			CMove a ((const char*)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
			CMove b = a;
//			v.push_back(CMove((const char*)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
		}

		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
//		std::cout << elapsed << std::endl;
		printf ("%-10f\n", elapsed);
	}

	{
		std::cout << "--move--" << std::endl;
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		std::vector<CMove> v;
		for (int i = 0; i < 1000; ++ i) {
			CMove a ((const char*)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
			CMove b(std::move(a));
//			v.push_back(std::move(CMove((const char*)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")));
		}

		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
//		std::cout << elapsed << std::endl;
		printf ("%-10f\n", elapsed);
	}

}

int main (void)
{
	f();
	f();
	f();

	return 0;
}
