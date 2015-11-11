#include "SacRec.h"
#include <string>

int main() {
	std::string fname("temp1");
	SacRec sac, sac_am; 
	sac.LoadTXT(fname);
	sac.ToAm( sac_am );
	sac_am.Dump(fname+"_freq.txt");
	return 0;
}
