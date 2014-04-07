#include "thread.h"
#include <process.h>

Thread::Thread()
{
	m_hThread = NULL;
}

Thread::~Thread()
{
	if(m_hThread != NULL)
		stop();
}

void Thread::start()
{
	unsigned int dwThreadId;
	m_hThread = (unsigned long*)::_beginthreadex(NULL,0,threadProc,
		                            (Thread*)this,0,&dwThreadId);
	setPriority(THREAD_PRIORITY_NORMAL);
}
void Thread::stop()
{
	if (m_hThread == NULL)
		return;
	::WaitForSingleObject(m_hThread,INFINITE);
	::CloseHandle(m_hThread);
	m_hThread = NULL;
}
void Thread::setPriority(int aPriority)
{
	if (m_hThread == NULL)
		return;

	::SetThreadPriority(m_hThread,aPriority);
}
void Thread::suspend()
{
	if (m_hThread == NULL)
		return ;
	::SuspendThread(m_hThread);
}
void Thread::resume()
{
	if (m_hThread == NULL)
		return ;
	::ResumeThread(m_hThread);
}
unsigned long* Thread::getHandle()
{
	return m_hThread;
}
unsigned int Thread::threadProc(void* param)
{
	Thread* tp = (Thread*)param;
	tp->run();
	return 0;
}