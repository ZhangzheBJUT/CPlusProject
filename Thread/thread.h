#ifndef _THREAD_H
#define _THREAD_H

#include <windows.h>

class Thread
{
public:
	virtual ~Thread();
	void start();
	void stop();
	void suspend();
	void resume();
	void setPriority(int aPriority=THREAD_PRIORITY_NORMAL);
	void sleep(long aMillions);
	unsigned long* getHandle();

protected:
	Thread();

private:
	virtual void run()=0;
	unsigned long* m_hThread;
	static unsigned int _stdcall threadProc(void* param);
};
#endif