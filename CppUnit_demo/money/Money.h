#ifndef _MONKY_H
#define _MONKY_H

#include<iostream>
#include<string>
using namespace std;


class CMoney
{
public:
	CMoney(double amount,string currency):m_amount(amount),m_currency(currency)
	{
	}
	~CMoney()
	{
	}
	
	double GetAmount() const;
	string GetCurrency() const;
	bool operator==(const CMoney &other) const;
	bool operator!=(const CMoney &other) const;
	CMoney &operator+=(const CMoney &other);
private:
	double m_amount;
	string m_currency;
};
#endif
