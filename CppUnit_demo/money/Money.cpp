#include<iostream>
#include<string>
#include "Money.h"

using namespace std;

double CMoney::GetAmount() const  
{  
	  return m_amount;  
}  
  
string CMoney::GetCurrency() const  
{  
	  return m_currency;  
}  
  
bool CMoney::operator ==( const CMoney &other ) const  
{  
	  return ((m_amount == other.m_amount) &&  
			           (m_currency == other.m_currency));  
}  
  
bool CMoney::operator !=( const CMoney &other ) const  
{  
	  return !(*this == other);  
}  
  
CMoney &CMoney::operator +=( const CMoney &other )  
{  
	  if ( m_currency != other.m_currency )  
		        cout <<  "Incompatible moneys" << endl;  
	  m_amount += other.m_amount;  
      
	  return *this;  
}

