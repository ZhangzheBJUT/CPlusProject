/******************************************* 
  Money_Test.h 
  2014.8.19 

 ********************************************/  
#ifndef _MONEY_TEST_H  
#define _MONEY_TEST_H  

#include <cppunit/extensions/HelperMacros.h>  
#include "Money.h"  

class CMoneyTest:public CppUnit::TestFixture  
{  
	/*声明一个TestSuite*/  
	CPPUNIT_TEST_SUITE(CMoneyTest);  

	/*添加测试用例到TestSuite,定义新的测试用例都要在这里声明； 
	  如果此处未声明某个测试用例，程序编译和运行都不会报错 
	  仅仅是该测试用例不会被执行。 
	 */  
	CPPUNIT_TEST(testConstructor);  
	CPPUNIT_TEST(testOptorEqual);  
	CPPUNIT_TEST(testOptorNotEqual);  
	CPPUNIT_TEST(testOptorAdd);  

	/*TestSuite声明完成*/  
	CPPUNIT_TEST_SUITE_END();  

	public:  
	CMoneyTest(){}  
	/*初始化 */  
	void setUp();  
	/*清除动作 */  
	void tearDown();  

	/*test app in Money.cpp*/  
	/*test case */  
	void testConstructor();  
	void testOptorEqual();  
	void testOptorNotEqual();  
	void testOptorAdd();  
};
#endif
