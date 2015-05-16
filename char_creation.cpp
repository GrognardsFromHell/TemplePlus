#include "stdafx.h"
#include "common.h"
#include "char_creation.h"

struct CharCreationAddresses : AddressTable
{
	uint32_t * pointBuyPointsRemaining;
	CharCreationAddresses()
	{
		rebase(pointBuyPointsRemaining, 0x10C453F4);
	}

} addresses;

CharCreationSystem charCreationSys;

void CharCreationSystem::SetPointBuyPoints(int points)
{
	uint8_t pointsMaxPrint = (uint8_t)points;
	uint8_t *pointsMaxPrintBuffer = &pointsMaxPrint;
	write(0x1018B575 + 1, pointsMaxPrintBuffer, sizeof(uint8_t));
	write(0x1018B5FD + 2, pointsMaxPrintBuffer, sizeof(uint8_t));
	write(0x1018B52E + 6, pointsMaxPrintBuffer, sizeof(uint8_t));
	write(0x1018B9EF + 1, pointsMaxPrintBuffer, sizeof(uint8_t));
}