#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


using namespace std;



template <typename T>
class ITmpltHandler
{
public:
	virtual ~ITmpltHandler (void) {};

	virtual void onHandled (T arg) = 0;
};


template <typename T>
class CTmplt
{
public:
	explicit CTmplt (ITmpltHandler<T> *pHandler);
	virtual ~CTmplt (void);

	T get (void) const;
	void set (T arg);
	void proc (void) ;

private:
	T m;
	ITmpltHandler<T> *mpHandler;
};

template <typename T>
CTmplt<T>::CTmplt (ITmpltHandler<T> *pHandler)
	:mpHandler (NULL)
{
	if (pHandler) {
		mpHandler = pHandler;
	}
}

template <typename T>
CTmplt<T>::~CTmplt (void)
{
}

template <typename T>
T CTmplt<T>::get (void) const
{
	return m;	
}

template <typename T>
void CTmplt<T>::set (T arg)
{
	m = arg;
}

template <typename T>
void CTmplt<T>::proc (void)
{
	if (mpHandler) {
		mpHandler->onHandled (m);
	}
}


template <typename T>
class CHandler : public ITmpltHandler<T>
{
public:
	CHandler (void) {}
	virtual ~CHandler (void) {}

	void onHandled (T arg);
};

template <typename T>
void CHandler<T>::onHandled (T arg)
{
	printf ("%d\n", arg);
}


int main (void)
{

//	CTmplt<char*> ta;
//	ta.set ((char*)"aaaaa");
//	printf ("%s\n", ta.get());


	CHandler<int> *pc = new CHandler<int>();

	CTmplt<int> *pti = new CTmplt<int> (pc);
	pti->set (5);
	printf ("%d\n", pti->get());

	pti->proc();

	delete pc;
	delete pti;


	exit (EXIT_SUCCESS);
}
