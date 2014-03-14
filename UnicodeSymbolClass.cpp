#include "UnicodeSymbolClass.h"
#include <iostream>
using namespace std;

UnicodeSymbolClass::UnicodeSymbolClass()
{
	//number:０１２３４５６７８９
	// unicode: \uff10\uff11\uff12\uff13\uff14\uff15\uff16\uff17\uff18\uff19;
	UnicodeSymbol.insert(pair <int,char *> (0xff10,"0"));
	UnicodeSymbol.insert(pair <int,char *> (0xff11,"1"));
	UnicodeSymbol.insert(pair <int,char *>( 0xff12,"2"));
	UnicodeSymbol.insert(pair <int,char *> (0xff13,"3"));
	UnicodeSymbol.insert(pair <int,char *>(0xff14,"4"));
	UnicodeSymbol.insert(pair <int,char *>(0xff15,"5"));
	UnicodeSymbol.insert(pair <int,char *>(0xff16,"6"));
	UnicodeSymbol.insert(pair <int,char *>(0xff17,"7"));
	UnicodeSymbol.insert(pair <int,char *>(0xff18,"8"));
	UnicodeSymbol.insert(pair <int,char *>(0xff19,"9"));

	//lowercase  a-z
	UnicodeSymbol.insert(pair<int,char *>(0xff41,"a"));
	UnicodeSymbol.insert(pair<int,char *>(0xff42,"b"));
	UnicodeSymbol.insert(pair<int,char *>(0xff43,"c"));
	UnicodeSymbol.insert(pair<int,char *>(0xff44,"d"));
	UnicodeSymbol.insert(pair<int,char *>(0xff45,"e"));
	UnicodeSymbol.insert(pair<int,char *>(0xff46,"f"));
	UnicodeSymbol.insert(pair<int,char *>(0xff47,"g"));
	UnicodeSymbol.insert(pair<int,char *>(0xff48,"h"));
	UnicodeSymbol.insert(pair<int,char *>(0xff49,"i"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4a,"j"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4b,"k"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4c,"l"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4d,"m"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4e,"n"));
	UnicodeSymbol.insert(pair<int,char *>(0xff4f,"o"));
	UnicodeSymbol.insert(pair<int,char *>(0xff50,"p"));
	UnicodeSymbol.insert(pair<int,char *>(0xff51,"q"));
	UnicodeSymbol.insert(pair<int,char *>(0xff52,"r"));
	UnicodeSymbol.insert(pair<int,char *>(0xff53,"s"));
	UnicodeSymbol.insert(pair<int,char *>(0xff54,"t"));
	UnicodeSymbol.insert(pair<int,char *>(0xff55,"u"));
	UnicodeSymbol.insert(pair<int,char *>(0xff56,"v"));
	UnicodeSymbol.insert(pair<int,char *>(0xff57,"w"));
	UnicodeSymbol.insert(pair<int,char *>(0xff58,"x"));
	UnicodeSymbol.insert(pair<int,char *>(0xff59,"y"));
	UnicodeSymbol.insert(pair<int,char *>(0xff5a,"z"));

	//upper case: A-Z
	UnicodeSymbol.insert(pair<int,char *>(0xff21,"A"));
	UnicodeSymbol.insert(pair<int,char *>(0xff22,"B"));
	UnicodeSymbol.insert(pair<int,char *>(0xff23,"C"));
	UnicodeSymbol.insert(pair<int,char *>(0xff24,"D"));
	UnicodeSymbol.insert(pair<int,char *>(0xff25,"E"));
	UnicodeSymbol.insert(pair<int,char *>(0xff26,"F"));
	UnicodeSymbol.insert(pair<int,char *>(0xff27,"G"));
	UnicodeSymbol.insert(pair<int,char *>(0xff28,"H"));
	UnicodeSymbol.insert(pair<int,char *>(0xff29,"I"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2a,"J"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2b,"K"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2c,"L"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2d,"M"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2e,"N"));
	UnicodeSymbol.insert(pair<int,char *>(0xff2f,"O"));
	UnicodeSymbol.insert(pair<int,char *>(0xff30,"P"));
	UnicodeSymbol.insert(pair<int,char *>(0xff31,"Q"));
	UnicodeSymbol.insert(pair<int,char *>(0xff32,"R"));
	UnicodeSymbol.insert(pair<int,char *>(0xff33,"S"));
	UnicodeSymbol.insert(pair<int,char *>(0xff34,"T"));
	UnicodeSymbol.insert(pair<int,char *>(0xff35,"U"));
	UnicodeSymbol.insert(pair<int,char *>(0xff36,"V"));
	UnicodeSymbol.insert(pair<int,char *>(0xff37,"W"));
	UnicodeSymbol.insert(pair<int,char *>(0xff38,"X"));
	UnicodeSymbol.insert(pair<int,char *>(0xff39,"Y"));
	UnicodeSymbol.insert(pair<int,char *>(0xff3a,"Z"));

	// αβγδεζηθ
	//\u03b1\u03b2\u03b3\u03b4\u03b5\u03b6\u03b7\u03b8
	UnicodeSymbol.insert(pair<int,char *>(0x03b1,"α"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b2,"β"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b3,"γ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b4,"δ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b5,"ε"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b6,"ζ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b7,"η"));
	UnicodeSymbol.insert(pair<int,char *>(0x03b8,"θ"));
	// λμξπρστφψωΓΔΘΦΨΩ
	// \u03bb\u03bc\u03be\u03c0\u03c1\u03c3\u03c4\u03c6\u03c8\u03c9\u0393\u0394\u0398\u03a6\u03a8\u03a9
	UnicodeSymbol.insert(pair<int,char *>(0x03bb,"λ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03bc,"μ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03be,"ξ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c0,"π"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c1,"ρ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c3,"σ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c4,"τ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c6,"φ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c8,"ψ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03c9,"ω"));
	UnicodeSymbol.insert(pair<int,char *>(0x0393,"Γ"));
	UnicodeSymbol.insert(pair<int,char *>(0x0394,"Δ"));
	UnicodeSymbol.insert(pair<int,char *>(0x0398,"Θ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03a6,"Φ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03a8,"Ψ"));
	UnicodeSymbol.insert(pair<int,char *>(0x03a9,"Ω"));
	
	//math  +-±×÷/√()<>≤≥
	//\u002b\u002d\u00b1\u00d7\u00f7\u002f\u221a\u0028\u0029\u003c\u003e\u2264\u2265
	UnicodeSymbol.insert(pair<int,char *>(0x002b,"+"));
	UnicodeSymbol.insert(pair<int,char *>(0x002d,"-"));
	UnicodeSymbol.insert(pair<int,char *>(0x00b1,"±"));
	UnicodeSymbol.insert(pair<int,char *>(0x00d7,"×"));
	UnicodeSymbol.insert(pair<int,char *>(0x00f7,"÷"));
	UnicodeSymbol.insert(pair<int,char *>(0x2215,"/"));
	UnicodeSymbol.insert(pair<int,char *>(0x221a,"√"));
	UnicodeSymbol.insert(pair<int,char *>(0x0028,"("));
	UnicodeSymbol.insert(pair<int,char *>(0x0029,")"));
	UnicodeSymbol.insert(pair<int,char *>(0x003c,"<"));
	UnicodeSymbol.insert(pair<int,char *>(0x003e,">"));
	UnicodeSymbol.insert(pair<int,char *>(0x2264,"≤"));
	UnicodeSymbol.insert(pair<int,char *>(0x2265,"≥"));

	//[]{}|=〈〉∑∏∫∮≠≈→．＇！∞∪∩∈⊥∥∠％
	//\u005b\u005d\u007b\u007d\u007c\u003d\u3008\u3009\u2211
	//\u220f\u222b\u222e\u2260\u2248\u2192\uff0e\uff07\uff01\u221e\u222a\u2229\u2208\u22a5\u2225\u2220\uff05
	UnicodeSymbol.insert(pair<int,char *>(0x005b,"["));
	UnicodeSymbol.insert(pair<int,char *>(0x005d,"]"));
	UnicodeSymbol.insert(pair<int,char *>(0x007b,"{"));
	UnicodeSymbol.insert(pair<int,char *>(0x007d,"}"));
	UnicodeSymbol.insert(pair<int,char *>(0x007c,"|"));
	UnicodeSymbol.insert(pair<int,char *>(0x003d,"="));
	UnicodeSymbol.insert(pair<int,char *>(0x3008,"〈"));
	UnicodeSymbol.insert(pair<int,char *>(0x3009,"〉"));
	UnicodeSymbol.insert(pair<int,char *>(0x2211,"∑"));
	UnicodeSymbol.insert(pair<int,char *>(0x220f,"∏"));
	UnicodeSymbol.insert(pair<int,char *>(0x222b,"∫"));
	UnicodeSymbol.insert(pair<int,char *>(0x222e,"∮"));
	UnicodeSymbol.insert(pair<int,char *>(0x2260,"≠"));
	UnicodeSymbol.insert(pair<int,char *>(0x2248,"≈"));
	UnicodeSymbol.insert(pair<int,char *>(0x2192,"→"));
	UnicodeSymbol.insert(pair<int,char *>(0xff0e,"．"));
	UnicodeSymbol.insert(pair<int,char *>(0xff07,"＇"));
	UnicodeSymbol.insert(pair<int,char *>(0xff01,"！"));
	UnicodeSymbol.insert(pair<int,char *>(0x221e,"∞"));
	UnicodeSymbol.insert(pair<int,char *>(0x222a,"∪"));
	UnicodeSymbol.insert(pair<int,char *>(0x2229,"∩"));
	UnicodeSymbol.insert(pair<int,char *>(0x2208,"∈"));
	UnicodeSymbol.insert(pair<int,char *>(0x22a5,"⊥"));
	UnicodeSymbol.insert(pair<int,char *>(0x2225,"∥"));
	UnicodeSymbol.insert(pair<int,char *>(0x2220,"∠"));
	UnicodeSymbol.insert(pair<int,char *>(0xff05,"％"));
	UnicodeSymbol.insert(pair<int,char *>(0x0000," "));
}

void UnicodeSymbolClass::printUnicodeSymbol(int unicode)
{
	map<int, char *>::iterator UnicodeSymbolIterator;
	UnicodeSymbolIterator = UnicodeSymbol.find(unicode);
	if( UnicodeSymbolIterator == UnicodeSymbol.end() )
	{
		cout<<"No corresponding unicode symbol here!"<<endl;
	}
	else
	{
		cout<<UnicodeSymbolIterator->second;
	}
}

void UnicodeSymbolClass::printAllHereUnicodeSymbols()
{
	map<int, char *>::iterator UnicodeSymbolIterator;
	for(UnicodeSymbolIterator = UnicodeSymbol.begin(); UnicodeSymbolIterator != UnicodeSymbol.end(); UnicodeSymbolIterator++)
	{
		cout<<UnicodeSymbolIterator->second<<endl;
	}
}
