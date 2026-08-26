#pragma once

/*
 * Floppy//144 Site projection.
 *
 * Projection changes only how the reconstructed Site is presented.
 * World geometry, collision, player position and persistent Site state remain
 * unchanged when switching projection.
 */

typedef enum Floppy144Projection
{
    FLOPPY144_PROJECTION_2D = 0,
    FLOPPY144_PROJECTION_ISOMETRIC,

    FLOPPY144_PROJECTION_COUNT
}
Floppy144Projection;
