#include "stdafx.h"
#include "externalizer.h"



static void DumpConditionStructs() {
	logger->info("Dumping Conditions Structs");
	CondStruct ** classes = ConditionArrayClasses.ptr();
	

	//CondStructArrayDump()

};



void CondStructArrayDump(CondStruct** array, char * arrayName, uint32_t arraySize)
{
	CondStruct * cond;
	for (uint32_t i = 0; i < arraySize; i++)
	{
		cond = array[i];
		if (cond)
		{
			uint32_t dummy = 1;
			// dump
			// 
		}

	}
};




static PythonDebugFunc pyConditionDebug("dump_CondStructs", &DumpConditionStructs);