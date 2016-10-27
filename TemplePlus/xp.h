#include "common.h"
const uint32_t XPTABLE_MAXLEVEL = 50;
const int CRMIN = -2; // equiv to CR 1/4  (next ones are CR 1/3, CR 1/2, CR 1, CR 2, CR 3,...
const int CRMAX = 50;
const int CRCOUNT = CRMAX - CRMIN + 1; // number of challenge ratings for our table
#define CR_KILLED_TABLE_SIZE 23

class XPAward{
public:
	int XPAwardTable[XPTABLE_MAXLEVEL][CRCOUNT];

	BOOL XpGainProcess(objHndl handle, int xpGainRaw);
	int GetMulticlassXpReductionPercent(objHndl handle);

	XPAward();
};
