#include "floppy144_interaction_engine.h"
#include <stddef.h>
#include <string.h>
Floppy144InteractionId Floppy144InteractionForPhysicalSource(const char *pszPhysicalSource){uint32_t uIndex;int32_t nOrdinal=0;if(!pszPhysicalSource)return FLOPPY144_INTERACTION_COUNT;for(uIndex=0;uIndex<Floppy144GameDataRecordCount();++uIndex){const Floppy144DataRecord*pRecord=Floppy144GameDataRecordAt(uIndex);if(!pRecord||pRecord->eKind!=FLOPPY144_DATA_INTERACTION)continue;if(pRecord->pszB&&strcmp(pRecord->pszB,pszPhysicalSource)==0)return(Floppy144InteractionId)nOrdinal;++nOrdinal;}return FLOPPY144_INTERACTION_COUNT;}
bool Floppy144InteractionCanRun(const Floppy144RunState*pState,Floppy144InteractionId eInteraction){return Floppy144GameDataInteractionCanRun(pState,eInteraction);}
bool Floppy144InteractionTryRun(Floppy144WorldState*pWorld,Floppy144RunState*pState,Floppy144InteractionId eInteraction){return Floppy144GameDataInteractionTryRun(pWorld,pState,eInteraction);}
