/*
 * FLOPPY//144 persistent operator-profile screen.
 *
 * The view consumes only Floppy144DiscoveryProfile. Current recovery-session
 * state is intentionally absent from this interface so transient run data
 * cannot be presented as permanent operator history.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_profile.h"
#include "floppy144_profile_edit.h"

/*
 * Draw the persistent GDR operator record into the existing software surface.
 */
void Floppy144ProfileViewDraw(
    Floppy144Surface *surface,
    const Floppy144DiscoveryProfile *profile,
    const Floppy144ProfileNameEditState *name_edit
);
