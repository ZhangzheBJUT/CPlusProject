#include "myThread.h"
#include <iostream>

using std::cout;
using std::endl;

MyThread::MyThread()
{
}

MyThread::~MyThread()
{

}
void MyThread::run()
{
	for (int i=0; i!=10; i++)
	{
		cout << "Thread Run....." << endl;
	}
	cout << "Thread stop!!	" << endl;
}