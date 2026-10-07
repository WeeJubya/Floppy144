/*
 * FLOPPY//144 Stage 4C persistent operator-profile screen regression.
 *
 * The renderer is tested as a pure view over persistent profile state. Stubs
 * satisfy the profile module's run-state queries without bringing a live
 * recovery session or platform environment into this test.
 */

#include "floppy144_profile.h"
#include "floppy144_profile_view.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U

static int failures;

/*
 * Record one profile-screen regression expectation.
 */
static void Expect(
    bool condition,
    const char *label
)
{
    if(!condition)
    {
        ++failures;

        printf(
            "FAIL: %s\n",
            label
        );
    }
}

/*
 * Provide the minimal collection query required by floppy144_profile.c.
 */
bool Floppy144RunStateCollectionRestored(
    const Floppy144RunState *state,
    Floppy144CollectionId collection
)
{
    uint32_t index;
    uint32_t bit;

    if(
        state == NULL ||
        collection < 0 ||
        collection >= FLOPPY144_COLLECTION_COUNT
    )
    {
        return false;
    }

    index =
        (uint32_t)collection /
        FLOPPY144_RUN_WORD_BITS;

    bit =
        (uint32_t)collection %
        FLOPPY144_RUN_WORD_BITS;

    return
        (
            state->collections[index] &
            (
                1U <<
                bit
            )
        ) != 0U;
}

/*
 * Provide the minimal evidence query required by floppy144_profile.c.
 */
bool Floppy144RunStateEvidenceEstablished(
    const Floppy144RunState *state,
    Floppy144EvidenceId evidence
)
{
    uint32_t index;
    uint32_t bit;

    if(
        state == NULL ||
        evidence < 0 ||
        evidence >= FLOPPY144_EVIDENCE_COUNT
    )
    {
        return false;
    }

    index =
        (uint32_t)evidence /
        FLOPPY144_RUN_WORD_BITS;

    bit =
        (uint32_t)evidence %
        FLOPPY144_RUN_WORD_BITS;

    return
        (
            state->evidence[index] &
            (
                1U <<
                bit
            )
        ) != 0U;
}

/*
 * Supply a stable recovered-data value for completion-history presentation.
 */
uint32_t Floppy144RunStateRecoveredKb(
    const Floppy144RunState *state
)
{
    if(state == NULL)
    {
        return 0U;
    }

    return 768U;
}

/*
 * Mark one collection bit in a synthetic run state.
 */
static void TestSetRunCollection(
    Floppy144RunState *state,
    uint32_t collection
)
{
    uint32_t index;
    uint32_t bit;

    if(
        state == NULL ||
        collection >= (uint32_t)FLOPPY144_COLLECTION_COUNT
    )
    {
        return;
    }

    index =
        collection /
        FLOPPY144_RUN_WORD_BITS;

    bit =
        collection %
        FLOPPY144_RUN_WORD_BITS;

    state->collections[index] |=
        1U <<
        bit;
}

/*
 * Mark one evidence bit in a synthetic run state.
 */
static void TestSetRunEvidence(
    Floppy144RunState *state,
    uint32_t evidence
)
{
    uint32_t index;
    uint32_t bit;

    if(
        state == NULL ||
        evidence >= (uint32_t)FLOPPY144_EVIDENCE_COUNT
    )
    {
        return;
    }

    index =
        evidence /
        FLOPPY144_RUN_WORD_BITS;

    bit =
        evidence %
        FLOPPY144_RUN_WORD_BITS;

    state->evidence[index] |=
        1U <<
        bit;
}

/*
 * Produce a deterministic hash of one rendered framebuffer.
 */
static uint32_t TestSurfaceHash(
    const uint32_t *pixels,
    uint32_t count
)
{
    uint32_t hash;
    uint32_t index;

    if(pixels == NULL)
    {
        return 0U;
    }

    hash =
        2166136261U;

    for(
        index = 0U;
        index < count;
        ++index
    )
    {
        hash ^=
            pixels[index];

        hash *=
            16777619U;
    }

    return hash;
}

/*
 * Draw one profile and return the resulting framebuffer hash.
 */
static uint32_t TestDrawProfile(
    const Floppy144DiscoveryProfile *profile,
    uint32_t *pixels
)
{
    Floppy144Surface surface;

    memset(
        pixels,
        0xCD,
        TEST_WIDTH *
        TEST_HEIGHT *
        sizeof(uint32_t)
    );

    surface.pixels =
        pixels;

    surface.width =
        TEST_WIDTH;

    surface.height =
        TEST_HEIGHT;

    Floppy144ProfileViewDraw(
        &surface,
        profile,
        NULL
    );

    return TestSurfaceHash(
        pixels,
        TEST_WIDTH *
        TEST_HEIGHT
    );
}

/*
 * Verify the screen handles a completely fresh persistent profile safely.
 */
static void TestFreshProfile(
    uint32_t *pixels
)
{
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile before;
    uint32_t first_hash;
    uint32_t second_hash;

    Floppy144DiscoveryProfileReset(
        &profile
    );

    before =
        profile;

    Expect(
        profile.operator_name[0] == '\0',
        "fresh profile has no operator name"
    );

    Expect(
        profile.recovery_sessions_begun == 0U &&
        profile.completed_recoveries == 0U,
        "fresh profile has zero recovery history"
    );

    Expect(
        Floppy144DiscoveryProfileCollectionsEverRestoredCount(
            &profile
        ) == 0U,
        "fresh profile has zero historical collections"
    );

    Expect(
        Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
            &profile
        ) == 0U,
        "fresh profile has zero historical evidence"
    );

    first_hash =
        TestDrawProfile(
            &profile,
            pixels
        );

    second_hash =
        TestDrawProfile(
            &profile,
            pixels
        );

    Expect(
        first_hash != 0U,
        "fresh profile renders a visible record"
    );

    Expect(
        first_hash == second_hash,
        "repeated fresh-profile display is deterministic"
    );

    Expect(
        memcmp(
            &profile,
            &before,
            sizeof(profile)
        ) == 0,
        "opening profile repeatedly does not mutate fresh history"
    );
}

/*
 * Verify real persistent fields and cumulative counters drive the display.
 */
static void TestExistingProfile(
    uint32_t *pixels
)
{
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile before;
    Floppy144RunState completed_run;
    uint32_t completed_hash;
    uint32_t without_completion_hash;
    uint32_t index;

    Floppy144DiscoveryProfileReset(
        &profile
    );

    Expect(
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            "GLYNN WILLIAMS"
        ),
        "existing profile accepts operator name"
    );

    Expect(
        Floppy144DiscoveryProfileSetBodyStyle(
            &profile,
            FLOPPY144_OPERATOR_BODY_STYLE_B
        ),
        "existing profile accepts body style"
    );

    for(
        index = 0U;
        index < 4U;
        ++index
    )
    {
        Floppy144DiscoveryProfileBeginRecovery(
            &profile
        );
    }

    Expect(
        Floppy144DiscoveryProfileRecordCollection(
            &profile,
            (Floppy144CollectionId)0
        ),
        "historical collection zero records"
    );

    Expect(
        Floppy144DiscoveryProfileRecordCollection(
            &profile,
            (Floppy144CollectionId)1
        ),
        "historical collection one records"
    );

    Expect(
        Floppy144DiscoveryProfileRecordCollection(
            &profile,
            (Floppy144CollectionId)(
                FLOPPY144_COLLECTION_COUNT -
                1
            )
        ),
        "historical final collection records"
    );

    Expect(
        Floppy144DiscoveryProfileRecordEvidence(
            &profile,
            (Floppy144EvidenceId)0
        ),
        "historical evidence zero records"
    );

    Expect(
        Floppy144DiscoveryProfileRecordEvidence(
            &profile,
            (Floppy144EvidenceId)(
                FLOPPY144_EVIDENCE_COUNT -
                1
            )
        ),
        "historical final evidence records"
    );

    Expect(
        Floppy144DiscoveryProfileCollectionsEverRestoredCount(
            &profile
        ) == 3U,
        "profile reports cumulative collection count"
    );

    Expect(
        Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
            &profile
        ) == 2U,
        "profile reports cumulative evidence count"
    );

    memset(
        &completed_run,
        0,
        sizeof(completed_run)
    );

    TestSetRunCollection(
        &completed_run,
        0U
    );

    TestSetRunCollection(
        &completed_run,
        1U
    );

    TestSetRunEvidence(
        &completed_run,
        0U
    );

    TestSetRunEvidence(
        &completed_run,
        1U
    );

    Expect(
        Floppy144DiscoveryProfileMergeRunState(
            &profile,
            &completed_run
        ),
        "completed run merges newly discovered evidence"
    );

    Expect(
        Floppy144DiscoveryProfileRecordCompletion(
            &profile,
            &completed_run,
            true,
            false
        ),
        "completed run records persistent completion history"
    );

    Expect(
        profile.completed_recoveries == 1U,
        "completed-recovery counter increments"
    );

    Expect(
        profile.latest_completion_evidence_percent ==
        (uint8_t)(
            2U *
            100U /
            (uint32_t)FLOPPY144_EVIDENCE_COUNT
        ),
        "latest completion stores evidence percentage"
    );

    Expect(
        profile.latest_completion_recovered_kb == 768U,
        "latest completion stores recovered data"
    );

    before =
        profile;

    completed_hash =
        TestDrawProfile(
            &profile,
            pixels
        );

    Expect(
        memcmp(
            &profile,
            &before,
            sizeof(profile)
        ) == 0,
        "drawing existing profile does not alter persistent history"
    );

    profile.completed_recoveries =
        0U;

    profile.latest_completion_evidence_percent =
        0U;

    profile.latest_completion_flags =
        0U;

    profile.latest_completion_recovered_kb =
        0U;

    without_completion_hash =
        TestDrawProfile(
            &profile,
            pixels
        );

    Expect(
        completed_hash != without_completion_hash,
        "completed-run history changes the persistent profile display"
    );
}

/*
 * Run the complete Stage 4C profile-screen regression.
 */
int main(void)
{
    uint32_t *pixels;

    pixels =
        (uint32_t *)malloc(
            TEST_WIDTH *
            TEST_HEIGHT *
            sizeof(uint32_t)
        );

    if(pixels == NULL)
    {
        printf(
            "FAIL: framebuffer allocation failed\n"
        );

        return 2;
    }

    TestFreshProfile(
        pixels
    );

    TestExistingProfile(
        pixels
    );

    free(
        pixels
    );

    if(failures != 0)
    {
        printf(
            "STAGE 4C PROFILE SCREEN TESTS: FAIL (%d)\n",
            failures
        );

        return 1;
    }

    printf(
        "STAGE 4C PROFILE SCREEN TESTS: PASS\n"
    );

    return 0;
}
