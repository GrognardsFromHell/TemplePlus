#pragma once

#include "stdafx.h"
#include "addresses.h"
#include "tig_mes.h"


uint32_t IsCombatActive();
uint32_t Combat_GetMesfileIdx_CombatMes();

extern GlobalPrimitive<MesHandle, 0x10AA8408> combatsysCombatMesfileIdx;