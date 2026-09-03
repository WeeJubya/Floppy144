/*
 * Floppy//144 - Site-space object interaction bridge
 *
 * Connects existing registered object behaviour to projection-neutral
 * 100x100 Site coordinates.
 *
 * The old object registry still contains technical-slice pixel positions.
 * This module deliberately ignores those positions and supplies Site-space
 * locations only for interactions that have migrated to the exact Site.
 */

#pragma once

#include "floppy144_object.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"

#include <stdbool.h>

/*
 * Return the best eligible Site interaction at the player's persistent
 * Site position.
 *
 * Existing object interaction definitions remain responsible for actions,
 * prompts, priority, notices and effects.
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
