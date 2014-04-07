#ifndef _MYTHREAD_H
#define _MYTHREAD_H


#include "thread.h"

class MyThread : public Thread
{
public:
	MyThread();
	virtual ~MyThread();
private:
	void run();
};
#endif