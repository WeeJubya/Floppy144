#pragma once
#include "floppy144_game_data.h"
#include <stdbool.h>
Floppy144InteractionId Floppy144InteractionForPhysicalSource(const char *pszPhysicalSource);
bool Floppy144InteractionCanRun(const Floppy144RunState *pState,Floppy144InteractionId eInteraction);
bool Floppy144InteractionTryRun(Floppy144WorldState *pWorld,Floppy144RunState *pState,Floppy144InteractionId eInteraction);
