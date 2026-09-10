/*
 * Floppy//144 trigger engine implementation
 */

#include "floppy144_trigger_engine.h"

#include <stddef.h>

#define FLOPPY144_ARRAY_COUNT(values)                              \
((uint32_t)(sizeof(values) / sizeof((values)[0])))


/*
 * T-001 - Disk Recovery Index
 *
 * Establishes the minimum reconstructed Site after the player opens the
 * recovered Disk Recovery Index.
 */
static const Floppy144Effect floppy144_t001_effects[] =
{
    {
        FLOPPY144_EFFECT_RECONSTRUCT_ROOM,
        (uint32_t)FLOPPY144_ROOM_RECEPTION
    },

    {
        FLOPPY144_EFFECT_RECONSTRUCT_ROOM,
        (uint32_t)FLOPPY144_ROOM_CORRIDOR
    }
};

/*
 * T-002 - Site Establishment Register
 *
 * Establishes the Main Office as reconstructed Site geometry.
 *
 * HR-01 itself is only available after T-001, so the authored route already
 * guarantees the design prerequisite that DR-01 has been restored.
 */
static const Floppy144Effect floppy144_t002_effects[] =
{
    {
        FLOPPY144_EFFECT_RECONSTRUCT_ROOM,
        (uint32_t)FLOPPY144_ROOM_MAIN_OFFICE
    }
};

/*
 * T-003 - Main Office Staffing Allocation
 *
 * T-002 establishes the Main Office. T-003 then establishes access from
 * Reception and restores the staffing-allocation physical material.
 *
 * Reception <-> Main Office access is derived directly from the persistent
 * T-003 trigger bit in RunState.
 */
static const Floppy144Effect floppy144_t003_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P008_DESK_NUMBER_LABELS
    },

    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P009_TEMP_DESK_ALLOCATION_SLIP
    },

    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P010_HOT_DESKING_NOTE
    }
};

/*
 * T-004 - Facilities Access Authorisation
 *
 * Once Main Office access has been established, this reconstructs the
 * Facilities room. Physical access still depends on reaching the Corridor.
 */
static const Floppy144Effect floppy144_t004_effects[] =
{
    {
        FLOPPY144_EFFECT_RECONSTRUCT_ROOM,
        (uint32_t)FLOPPY144_ROOM_FACILITIES
    }
};

/*
 * T-006 - Suppression Control Panel Service Note
 */
static const Floppy144Effect floppy144_t006_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_SUPPRESSION_CONTROL_PANEL
    },

    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P013_PANEL_SERVICE_LABEL
    }
};

/*
 * T-007 - Manual Discharge Circuit Test Record
 */
static const Floppy144Effect floppy144_t007_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P032_SUPPRESSION_ZONE_DIAGRAM
    }
};

/*
 * T-008 - Final Decommissioning Schedule
 */
static const Floppy144Effect floppy144_t008_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P034_DECOMMISSIONING_TAG_SET
    },

    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P035_ASSET_TRANSFER_LABELS
    },

    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P036_CONTRACTOR_CLIPBOARD
    }
};

/*
 * T-009 - Retained Services Schedule
 */
static const Floppy144Effect floppy144_t009_effects[] =
{
    {
        FLOPPY144_EFFECT_REVEAL_OBJECT,
        (uint32_t)FLOPPY144_OBJECT_P033_FINAL_ISOLATION_REGISTER
    }
};

typedef struct Floppy144AuthoredTrigger
{
    Floppy144TriggerId trigger;
    Floppy144TriggerDefinition definition;
}
Floppy144AuthoredTrigger;


static const Floppy144AuthoredTrigger
floppy144_authored_triggers[] =
{
    {
        FLOPPY144_TRIGGER_T001,
        {
            FLOPPY144_TRIGGER_COUNT,
            floppy144_t001_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t001_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T002,
        {
            FLOPPY144_TRIGGER_COUNT,
            floppy144_t002_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t002_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T003,
        {
            FLOPPY144_TRIGGER_T002,
            floppy144_t003_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t003_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T004,
        {
            FLOPPY144_TRIGGER_T003,
            floppy144_t004_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t004_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T005,
        {
            FLOPPY144_TRIGGER_T004,
            NULL,
            0U
        }
    },

    {
        FLOPPY144_TRIGGER_T006,
        {
            FLOPPY144_TRIGGER_T005,
            floppy144_t006_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t006_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T007,
        {
            FLOPPY144_TRIGGER_T006,
            floppy144_t007_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t007_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T008,
        {
            FLOPPY144_TRIGGER_T005,
            floppy144_t008_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t008_effects
            )
        }
    },

    {
        FLOPPY144_TRIGGER_T009,
        {
            FLOPPY144_TRIGGER_T008,
            floppy144_t009_effects,
            FLOPPY144_ARRAY_COUNT(
                floppy144_t009_effects
            )
        }
    }
};

static const Floppy144TriggerDefinition
floppy144_trigger_definitions[] =
{
    #define FLOPPY144_TRIGGER(symbol, code)       \
    {                                             \
        FLOPPY144_TRIGGER_COUNT,                  \
        NULL,                                     \
        0U                                        \
    },

    #include "floppy144_triggers.def"

    #undef FLOPPY144_TRIGGER
};


typedef char Floppy144TriggerDefinitionCountCheck[
    (
        sizeof(floppy144_trigger_definitions) /
        sizeof(floppy144_trigger_definitions[0])
    ) ==
    FLOPPY144_TRIGGER_COUNT
    ? 1
    : -1
];


/*
 * Locate one trigger definition.
 */

const Floppy144TriggerDefinition *Floppy144TriggerGet
(
    Floppy144TriggerId trigger
)
{
    uint32_t authored_index;

    if(
        (uint32_t)trigger >=
        (uint32_t)FLOPPY144_TRIGGER_COUNT
    )
    {
        return NULL;
    }

    for(
        authored_index = 0U;
        authored_index <
        FLOPPY144_ARRAY_COUNT(
        floppy144_authored_triggers
    );
    ++authored_index
    )
    {
        const Floppy144AuthoredTrigger *authored =
        &floppy144_authored_triggers[
            authored_index
        ];

        if(authored->trigger == trigger)
        {
            return &authored->definition;
        }
    }

    return
    &floppy144_trigger_definitions[
        (uint32_t)trigger
    ];
}


/*
 * Test generic trigger eligibility.
 */

bool Floppy144TriggerCanFire
(
    const Floppy144RunState *run_state,
 Floppy144TriggerId trigger
)
{
    const Floppy144TriggerDefinition *definition;

    if(run_state == NULL)
    {
        return false;
    }

    definition =
    Floppy144TriggerGet(
        trigger
    );

    if(definition == NULL)
    {
        return false;
    }

    if(
        Floppy144RunStateTriggerFired(
            run_state,
            trigger
        )
    )
    {
        return false;
    }

    if(
        definition->prerequisite !=
        FLOPPY144_TRIGGER_COUNT &&
        !Floppy144RunStateTriggerFired(
            run_state,
            definition->prerequisite
        )
    )
    {
        return false;
    }

    return true;
}


/*
 * Fire one trigger exactly once and route its effects through the shared
 * generic effect engine.
 */

bool Floppy144TriggerTryFire
(
    Floppy144WorldState *world,
 Floppy144RunState *run_state,
 Floppy144TriggerId trigger
)
{
    const Floppy144TriggerDefinition *definition;

    if(
        world == NULL ||
        run_state == NULL ||
        !Floppy144TriggerCanFire(
            run_state,
            trigger
        )
    )
    {
        return false;
    }

    definition =
    Floppy144TriggerGet(
        trigger
    );

    if(definition == NULL)
    {
        return false;
    }

    if(
        !Floppy144RunStateFireTrigger(
            run_state,
            trigger
        )
    )
    {
        return false;
    }

    if(
        definition->effects != NULL &&
        definition->effect_count > 0U
    )
    {
        Floppy144ApplyEffects(
            world,
            run_state,
            definition->effects,
            definition->effect_count
        );
    }

    return true;
}
