#pragma once
/* Generic trigger facade. No T-xxx-specific function belongs here. */
#include "floppy144_game_data.h"
#include "floppy144_trigger.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct Floppy144TriggerDefinition{const char*pszStableId;uint32_t uConditionCount,uEffectCount;}Floppy144TriggerDefinition;
const Floppy144TriggerDefinition *Floppy144TriggerGet(Floppy144TriggerId eTrigger);
bool Floppy144TriggerCanFire(const Floppy144RunState*pState,Floppy144TriggerId eTrigger);
bool Floppy144TriggerTryFire(Floppy144WorldState*pWorld,Floppy144RunState*pState,Floppy144TriggerId eTrigger);
