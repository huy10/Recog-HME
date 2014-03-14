#ifndef __UNICODESYMBOLCLASS_H__
#define __UNOCODESYMBOLCLASS_H__
#include<iostream>
#include<map>
using namespace std;

class UnicodeSymbolClass
{

public:
	UnicodeSymbolClass();
	void printUnicodeSymbol(int unicode);
	void printAllHereUnicodeSymbols();
private:
	map<int, char *> UnicodeSymbol;
};


#endif
