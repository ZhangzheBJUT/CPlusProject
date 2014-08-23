/**************************************************** 
  Money_Test_Main.cpp 
  2014.8.19

 ********************************************************/  
#include <cppunit/extensions/TestFactoryRegistry.h>  
#include <cppunit/ui/text/TestRunner.h>  

int main()  
{  
	CppUnit::TextUi::TestRunner runner;  

	/*从注册的TestSuite获取特定的TestSuite, 
	  没有参数的话则获取未命名的TestSuite*/  
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry("alltest");  

	/*添加这个TestSuite到TestRunner中*/  
	runner.addTest(registry.makeTest());  

	/*运行测试*/  
	runner.run();  

	return 0;
}  
