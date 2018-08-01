//
// compile: g++ *cpp -lpthread
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "Utils.h"
#include "AsyncProcProxy.h"
#include "ImmSocketServiceCommon.h"


using namespace std;
using namespace ImmSocketService;

template <typename T>
class CTest : public IAsyncHandler<T>
{
public:
	CTest (void) {}
	virtual ~CTest (void) {}

	void onAsyncHandled (T arg) {
		printf ("%d\n", *arg);

		// !!!
		delete arg;
	}
};


int main (void)
{
	puts ("start main");

	int *p = new int();
	*p = 400; 

	CTest<int*> *ptest = new CTest<int*> ();
	ptest->deletable();

	CAsyncProcProxy<int*> *pproxy = new CAsyncProcProxy<int*> (5);
	pproxy->start ();
	pproxy->request (p, ptest);


	pause ();

	exit (EXIT_SUCCESS);
}
