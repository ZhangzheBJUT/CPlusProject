/************************************ 
  Money_Test.cpp 
  8.19

 ****************************************/  
#include "Money_Test.h"  
#include "Money.h"  
#include <string>  

using namespace std;  

/* 将该TestSuite注册到名字为“alltest”的TestSuite中，如果未定义会自动定义，也可以使用CPPUNIT_TEST_SUITE_REGISTRATION( MathTest );定义到全局未命名的TestSuite中 */  

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(CMoneyTest,"alltest");  

/*初始化动作*/  
void CMoneyTest::setUp()  
{  

}  

/*清除动作*/  
void CMoneyTest::tearDown()  
{  

}  

/*编写测试用例， 
  此处编写四个用例分别来测试CMoney类的四个成员函数*/  
/*test app in Money.cpp*/  

/*test constructor*/  
void CMoneyTest::testConstructor()  
{  
	double dNum = 22124.44;  
	string sCurrency = "DD";  
	CMoney MyMoney(dNum, sCurrency);  

	CPPUNIT_ASSERT_EQUAL(12.0, MyMoney.GetAmount());  
	CPPUNIT_ASSERT_EQUAL(sCurrency, MyMoney.GetCurrency());  

}  


/*test operator ==()*/  
void CMoneyTest::testOptorEqual()  
{  
	// Set up  
	const CMoney money123FF( 123, "FF" );  
	const CMoney money123USD( 123, "USD" );  
	const CMoney money12FF( 12, "FF" );  
	const CMoney money12USD( 12, "USD" );  

	// Process & Check  
	CPPUNIT_ASSERT(money123FF == money123FF);    // ==  
	CPPUNIT_ASSERT(!(money12FF == money123FF));     // != amount  
	CPPUNIT_ASSERT(!(money123USD == money123FF));   // != currency  
	CPPUNIT_ASSERT(!(money12USD == money123FF));     
	// != currency and != amount  

}  

/*test operator!=()*/  
void CMoneyTest::testOptorNotEqual()  
{  
	// Set up  
	const CMoney money123FF( 123, "FF" );  
	const CMoney money123USD( 123, "USD" );  
	const CMoney money12FF( 12, "FF" );  
	const CMoney money12USD( 12, "USD" );  

	// Process & Check  
	CPPUNIT_ASSERT(!(money123FF != money123FF));    // ==  
	CPPUNIT_ASSERT(money12FF != money123FF);     // != amount  
	CPPUNIT_ASSERT(money123USD != money123FF);   // != currency  
	CPPUNIT_ASSERT(money12USD != money123FF);      
	// != currency and != amount  

}  

/*test operator+=()*/  
void CMoneyTest::testOptorAdd()  
{  
	// Set up  
	const CMoney money12FF( 12, "FF" );  
	const CMoney expectedMoney( 135, "FF" );  

	// Process  
	CMoney money( 123, "FF" );  
	money += money12FF;  

	// Check  
	CPPUNIT_ASSERT( expectedMoney ==  money );           // add works  
}  
