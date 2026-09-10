#include "floppy144_collection_registry.h"
#include "floppy144_game_data.h"
#include <stddef.h>
#include <string.h>

/* Neutral procedural labels for catalogue entries without authored bodies. */
static const char *const floppy144_generated_generic_subjects[]={
    "ADMINISTRATIVE","OPERATIONAL","SITE","STAFF","ACCESS","TRANSFER",
    "MAINTENANCE","REVIEW","CLOSURE","EXCEPTION","VERIFICATION","RECOVERY"
};
const Floppy144CollectionDefinition floppy144_collection_definitions[FLOPPY144_COLLECTION_COUNT]={
#define FLOPPY144_COLLECTION(symbol,code_text,title_text,domain_value,reconstruction_percent_value,description_text,evidence_description_text,catalogue_record_count,catalogue_heading_text,catalogue_record_id_prefix,catalogue_subjects,catalogue_subject_count,catalogue_exact_titles,catalogue_record_number_base,catalogue_record_number_multiplier,catalogue_record_number_offset) {FLOPPY144_COLLECTION_##symbol,code_text,title_text,domain_value,reconstruction_percent_value,description_text,evidence_description_text,{catalogue_record_count,catalogue_heading_text,catalogue_record_id_prefix,catalogue_subjects,catalogue_subject_count,catalogue_exact_titles,catalogue_record_number_base,catalogue_record_number_multiplier,catalogue_record_number_offset}},
#include "floppy144_collections.def"
#undef FLOPPY144_COLLECTION
};
const Floppy144CollectionDefinition *Floppy144CollectionGet(Floppy144CollectionId eCollection){uint32_t u=(uint32_t)eCollection;if(u>=(uint32_t)FLOPPY144_COLLECTION_COUNT)u=(uint32_t)FLOPPY144_COLLECTION_DR01;return&floppy144_collection_definitions[u];}
Floppy144TriggerId Floppy144CollectionAvailabilityTrigger(Floppy144CollectionId eCollection){const Floppy144CollectionDefinition*p;uint32_t u;if((uint32_t)eCollection>=(uint32_t)FLOPPY144_COLLECTION_COUNT)return FLOPPY144_TRIGGER_COUNT;p=Floppy144CollectionGet(eCollection);for(u=0;u<Floppy144GameDataRecordCount();++u){const Floppy144DataRecord*r=Floppy144GameDataRecordAt(u);if(r&&r->eKind==FLOPPY144_DATA_TRIGGER_EFFECT&&r->pszA&&r->pszB&&strcmp(r->pszA,"ENABLE_COLLECTION")==0&&strcmp(r->pszB,p->code)==0)return Floppy144GameDataTriggerId(r->pszId);}return FLOPPY144_TRIGGER_COUNT;}
const char *Floppy144CollectionDomainText(Floppy144CollectionDomain eDomain){switch(eDomain){case FLOPPY144_COLLECTION_DOMAIN_DR:return"DR";case FLOPPY144_COLLECTION_DOMAIN_HR:return"HR";case FLOPPY144_COLLECTION_DOMAIN_FM:return"FM";case FLOPPY144_COLLECTION_DOMAIN_OS:return"OS";case FLOPPY144_COLLECTION_DOMAIN_TS:return"TS";default:return"UNKNOWN";}}
