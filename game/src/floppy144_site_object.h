/*
 * Floppy//144 - Site-space object interaction bridge
 *
 * Stage 3B.2 keeps projection-neutral Site targeting separate from the Win32
 * coordinator. Physical interaction targets are derived from generated game
 * data rather than from story-specific C branches.
 */

#pragma once

#include "floppy144_object.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Site actions exposed to presentation code.
 *
 * A nearby target may support both actions. A terminal desk can therefore be
 * accessed with A while any reconstructed physical material on the same desk
 * remains inspectable with I.
 */
#define FLOPPY144_SITE_ACTION_ACCESS   (1U << 0)
#define FLOPPY144_SITE_ACTION_INSPECT  (1U << 1)

/*
 * Data-driven physical inspection target.
 *
 * pszParentId always names the furniture/fixture whose Site rectangle is being
 * targeted. pszPhysicalItemId/name are NULL when that parent is present but no
 * child physical item is currently visible. eInteraction is COUNT when the
 * selected physical item is contextual rather than progression-bearing.
 */
typedef struct Floppy144SiteInspectionTarget
{
    const char *pszParentId;
    const char *pszPhysicalItemId;
    const char *pszPhysicalItemName;

    Floppy144InteractionId eInteraction;

    bool bInteractionAvailable;
    bool bInteractionCompleted;
}
Floppy144SiteInspectionTarget;

/*
 * Return the currently available Site control actions for the player's
 * position. This is shared by 2D/isometric presentation and by input routing.
 */
uint32_t Floppy144SiteAvailableActions(
    const Floppy144RunState *pState
);

/*
 * Resolve the GDR terminal reachable from the player's current position.
 * Returns false when no reconstructed-room terminal is within access range.
 */
bool Floppy144SiteAccessTerminalRoom(
    const Floppy144RunState *pState,
    Floppy144RoomId *pRoom
);

/*
 * Resolve the best data-bearing furniture/fixture near the player and, when
 * visible, the most useful physical child item on that parent.
 */
bool Floppy144SiteResolveInspectionTarget(
    const Floppy144RunState *pState,
    Floppy144SiteInspectionTarget *pTarget
);

/*
 * Runtime visibility contract for generated Site rectangles.
 *
 * Ordinary geometry requires its owning room to be reconstructed. Internal
 * boundaries require both endpoint rooms; OUTSIDE counts as permanently
 * reconstructed. Progression-controlled object geometry must also have been
 * revealed. Rendering, collision and passive labels share this decision.
 */
bool Floppy144SiteRectRuntimeVisible(
    const Floppy144RunState *pState,
    const Floppy144SiteRect *pRect
);

/*
 * Passive proximity label for ordinary Site geometry. Explicit interaction
 * notices always take precedence in the renderer. Locked doors identify
 * themselves; unlocked doors are deliberately silent.
 */
const char *Floppy144SiteContextLabel(
    const Floppy144RunState *pState
);

/*
 * Legacy technical-slice target resolver.
 *
 * Kept during Stage 3 migration because geometry-visibility compatibility
 * still uses the small old object registry. New Site gameplay must use the
 * data-driven access/inspection APIs above.
 */
Floppy144ObjectId Floppy144SiteInteractionTarget(
    const Floppy144RunState *state
);

/*
 * Test whether generated Site geometry linked to an existing registered
 * top-level object should currently be drawn.
 *
 * Geometry with no migrated registry object is ordinary scenery and returns
 * true. Child/content objects do not hide their parent furniture footprint.
 */
bool Floppy144SiteObjectGeometryVisible(
    const Floppy144RunState *state,
    const Floppy144SiteRect *rect
);
