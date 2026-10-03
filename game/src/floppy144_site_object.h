/*
 * Floppy//144 - Site-space object interaction bridge
 *
 * Stage 3B.2 keeps projection-neutral Site targeting separate from the Win32
 * coordinator. Physical interaction targets are derived from generated game
 * data rather than from story-specific C branches.
 */

#pragma once

#include "floppy144_object.h"
#include "floppy144_game_data.h"
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
 * Return the generated furniture/fixture parent which owns the current
 * proximity focus. The same one-unit focus drives the passive label and all
 * contextual actions, preventing neighbouring parents from contributing
 * different controls to one prompt.
 */
const char *Floppy144SiteFocusedParentId(
    const Floppy144RunState *pState
);

/*
 * Resolve an authored Access interaction owned by the currently focused
 * conventional door. The interaction is returned only while its prerequisites
 * are satisfied, so the footer and the A key expose the same player action.
 */
bool Floppy144SiteAccessInteractionNearby(
    const Floppy144RunState *pState,
    Floppy144InteractionId *pInteraction
);

/*
 * Return true when the player is standing beside any authored Site Directory
 * fixture in the current reconstructed room. Both Reception and Corridor use
 * the same generic path, so future directories gain the map action for free.
 */
bool Floppy144SiteDirectoryNearby(
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
 * Query whether one canonical physical item is currently visible.
 *
 * Ordinary contextual items appear with their reconstructed room. Items named
 * by REVEAL_PHYSICAL_ITEM effects remain hidden until that persistent effect
 * has fired. Container screens and Site targeting share this rule.
 */
bool Floppy144SitePhysicalItemVisible(
    const Floppy144RunState *pState,
    const Floppy144DataRecord *pPhysicalItem
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
 * Return the player-facing plaque on a nearby Corridor-facing door.
 *
 * This applies from the Corridor side only and is independent of lock state,
 * so office signage remains inspectable after a connection opens. pLocked is
 * optional and reports the current connection state for Inspect feedback.
 */
const char *Floppy144SiteCorridorDoorLabelNearby(
    const Floppy144RunState *pState,
    bool *pLocked
);

/*
 * Return true when a currently visible locked door is within normal Inspect
 * range. This remains the fallback for non-Corridor locked doors.
 */
bool Floppy144SiteLockedDoorNearby(
    const Floppy144RunState *pState
);

/*
 * Passive proximity label for ordinary Site geometry. Explicit interaction
 * notices always take precedence in the renderer. Corridor-facing doors show
 * their room plaque regardless of lock state; other doors identify themselves
 * only while locked.
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
