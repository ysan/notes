#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>


int main()
{
	void *pHandler;
	int (*pFunc)();
	
	pHandler = dlopen( "./lib/stub.so", RTLD_LAZY );
	if( pHandler == NULL ){
		fprintf( stderr, "%s", dlerror() );
		exit( EXIT_FAILURE );
	}

	pFunc = dlsym( pHandler, "stub" );
	if( pFunc == NULL ){
		fprintf( stderr, "%s\n", dlerror() );
		exit( EXIT_FAILURE );
	}

	(*pFunc)();

	dlclose( pHandler );

	exit( EXIT_SUCCESS );
}

