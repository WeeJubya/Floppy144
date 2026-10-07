/*
 * FLOPPY//144 S4C-08 bespoke completion presentation regression.
 *
 * The view is tested with deterministic stubs so this suite proves rendering
 * and navigation without altering or duplicating persistence semantics.
 */
#include "floppy144_completion_view.h"
#include "floppy144_game_data.h"

#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U
#define TEST_PIXELS (TEST_WIDTH*TEST_HEIGHT)

static int failures;
static uint32_t guarded_pixels[TEST_PIXELS+2U];
static uint32_t test_free_kb=57U;

static const char long_evidence_text[]=
    "Recovered evidence remains part of the frozen archival record and this deliberately long authored-style test line exists to exercise wrapping, scrolling and the completion ledger without changing gameplay state.";

static const char final_conclusion[]=
    "The cable-riser condition, work sequence and suppression diagnostics support the authored final reconstruction. These findings remain the final conclusion exposed by the completion record.";

static const Floppy144DataRecord test_records[]=
{
    {
        FLOPPY144_DATA_EVIDENCE,
        "E-001",
        "First evidence",
        NULL,
        NULL,
        "Core",
        long_evidence_text,
        "Act I",
        0,0,1,0,0,0,1
    },
    {
        FLOPPY144_DATA_EVIDENCE,
        "E-023",
        "Fallback final evidence",
        NULL,
        NULL,
        "Core resolution",
        final_conclusion,
        "Act III",
        0,0,23,0,0,0,1
    },
    {
        FLOPPY144_DATA_EVIDENCE,
        "E-002",
        "Second evidence",
        NULL,
        NULL,
        "Core",
        long_evidence_text,
        "Act II",
        0,0,2,0,0,0,1
    },
    {
        FLOPPY144_DATA_EVIDENCE,
        "E-003",
        "Third evidence",
        NULL,
        NULL,
        "Core",
        long_evidence_text,
        "Act II",
        0,0,3,0,0,0,1
    }
};

static void Expect(int condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

uint32_t Floppy144GameDataRecordCount(void)
{
    return (uint32_t)(sizeof(test_records)/sizeof(test_records[0]));
}

const Floppy144DataRecord *Floppy144GameDataRecordAt(uint32_t index)
{
    return
        index<Floppy144GameDataRecordCount()
        ? &test_records[index]
        : NULL;
}

Floppy144EvidenceId Floppy144GameDataEvidenceId(const char *id)
{
    if(id==NULL) return FLOPPY144_EVIDENCE_COUNT;
    if(strcmp(id,"E-001")==0) return (Floppy144EvidenceId)0;
    if(strcmp(id,"E-023")==0) return (Floppy144EvidenceId)1;
    if(strcmp(id,"E-002")==0) return (Floppy144EvidenceId)2;
    if(strcmp(id,"E-003")==0) return (Floppy144EvidenceId)3;
    return FLOPPY144_EVIDENCE_COUNT;
}

bool Floppy144RunStateCollectionRestored(
    const Floppy144RunState *state,
    Floppy144CollectionId collection
)
{
    uint32_t bit=(uint32_t)collection;
    if(state==NULL||bit>=(uint32_t)FLOPPY144_COLLECTION_COUNT) return false;
    return (state->collections[bit/32U]&(1U<<(bit%32U)))!=0U;
}

bool Floppy144RunStateEvidenceEstablished(
    const Floppy144RunState *state,
    Floppy144EvidenceId evidence
)
{
    uint32_t bit=(uint32_t)evidence;
    if(state==NULL||bit>=(uint32_t)FLOPPY144_EVIDENCE_COUNT) return false;
    return (state->evidence[bit/32U]&(1U<<(bit%32U)))!=0U;
}

uint32_t Floppy144RunStateFreeKb(const Floppy144RunState *state)
{
    return state!=NULL?test_free_kb:0U;
}

const char *Floppy144DiscoveryProfileOperatorName(
    const Floppy144DiscoveryProfile *profile
)
{
    return profile!=NULL?profile->operator_name:"";
}

static void SetCollection(Floppy144RunState *state,uint32_t index)
{
    state->collections[index/32U]|=1U<<(index%32U);
}

static void SetEvidence(Floppy144RunState *state,uint32_t index)
{
    state->evidence[index/32U]|=1U<<(index%32U);
}

static uint32_t HashPixels(const uint32_t *pixels,uint32_t count)
{
    uint32_t hash=2166136261U;
    uint32_t index;
    for(index=0U;index<count;++index)
    {
        hash^=pixels[index];
        hash*=16777619U;
    }
    return hash;
}

static uint32_t Draw(
    Floppy144CompletionViewState *view,
    const Floppy144RunState *run,
    const Floppy144DiscoveryProfile *profile,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    Floppy144Surface surface;

    memset(guarded_pixels,0,sizeof(guarded_pixels));
    guarded_pixels[0]=0x13579BDFU;
    guarded_pixels[TEST_PIXELS+1U]=0x2468ACE0U;

    surface.pixels=&guarded_pixels[1];
    surface.width=TEST_WIDTH;
    surface.height=TEST_HEIGHT;

    Floppy144CompletionViewDraw(
        &surface,
        view,
        run,
        profile,
        evidence_resolved,
        capacity_exhausted
    );

    Expect(
        guarded_pixels[0]==0x13579BDFU &&
        guarded_pixels[TEST_PIXELS+1U]==0x2468ACE0U,
        "completion view stays inside the 640x360 framebuffer"
    );

    return HashPixels(&guarded_pixels[1],TEST_PIXELS);
}

int main(void)
{
    Floppy144RunState run;
    Floppy144RunState run_before;
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile profile_before;
    Floppy144CompletionViewState view;
    Floppy144CompletionViewState view_before;
    uint32_t evidence_hash;
    uint32_t capacity_hash;
    uint32_t both_hash;
    uint32_t final_hash;
    uint32_t unresolved_final_hash;

    memset(&run,0,sizeof(run));
    memset(&profile,0,sizeof(profile));
    (void)snprintf(
        profile.operator_name,
        sizeof(profile.operator_name),
        "%s",
        "GLYNN WILLIAMS"
    );

    SetCollection(&run,0U);
    SetCollection(&run,2U);
    SetCollection(&run,7U);

    SetEvidence(&run,0U);
    SetEvidence(&run,2U);
    SetEvidence(&run,3U);
    SetEvidence(&run,4U);

    Expect(
        strcmp(
            Floppy144CompletionViewOutcomeText(true,false),
            "EVIDENCE RESOLVED"
        )==0,
        "evidence-first ending retains its distinct outcome"
    );
    Expect(
        strcmp(
            Floppy144CompletionViewOutcomeText(false,true),
            "RECOVERY CAPACITY EXHAUSTED"
        )==0,
        "capacity-first ending retains its distinct outcome"
    );
    Expect(
        strcmp(
            Floppy144CompletionViewOutcomeText(true,true),
            "EVIDENCE RESOLVED / CAPACITY EXHAUSTED"
        )==0,
        "simultaneous ending retains its distinct outcome"
    );

    Expect(
        strcmp(
            Floppy144CompletionViewStatusText(true,false),
            Floppy144CompletionViewStatusText(false,true)
        )!=0 &&
        strcmp(
            Floppy144CompletionViewStatusText(true,true),
            Floppy144CompletionViewStatusText(true,false)
        )!=0,
        "all supported ending flavours have distinct archival status text"
    );

    Expect(
        Floppy144CompletionViewCollectionsRestored(&run)==3U,
        "summary counts actual restored collection bits"
    );
    Expect(
        Floppy144CompletionViewEvidenceEstablished(&run)==4U,
        "summary counts actual established evidence bits"
    );
    Expect(
        Floppy144CompletionViewCapacityRemainingPercent(&run)==3U,
        "summary derives remaining capacity percentage from actual free KB"
    );

    Expect(
        !Floppy144CompletionViewCoreResolutionEstablished(&run) &&
        Floppy144CompletionViewFinalConclusionText(&run)==NULL,
        "unresolved capacity ending does not invent a final conclusion"
    );

    Floppy144CompletionViewReset(&view);
    Expect(
        Floppy144CompletionViewSelectedOption(&view)==
            FLOPPY144_COMPLETION_OPTION_FINAL_NOTE &&
        !Floppy144CompletionViewFinalNoteOpen(&view),
        "completion opens on summary with Final Note selected"
    );

    Floppy144CompletionViewMoveSelection(&view,-1);
    Expect(
        Floppy144CompletionViewSelectedOption(&view)==
            FLOPPY144_COMPLETION_OPTION_MAIN_MENU,
        "summary selection wraps upward"
    );
    Floppy144CompletionViewMoveSelection(&view,1);
    Expect(
        Floppy144CompletionViewSelectedOption(&view)==
            FLOPPY144_COMPLETION_OPTION_FINAL_NOTE,
        "summary selection wraps downward"
    );

    run_before=run;
    profile_before=profile;
    view_before=view;

    evidence_hash=Draw(&view,&run,&profile,true,false);
    capacity_hash=Draw(&view,&run,&profile,false,true);
    both_hash=Draw(&view,&run,&profile,true,true);

    Expect(
        evidence_hash!=0U &&
        capacity_hash!=0U &&
        both_hash!=0U,
        "all completion flavours render"
    );
    Expect(
        evidence_hash!=capacity_hash &&
        evidence_hash!=both_hash &&
        capacity_hash!=both_hash,
        "all completion flavours render distinctly"
    );
    Expect(
        memcmp(&run,&run_before,sizeof(run))==0 &&
        memcmp(&profile,&profile_before,sizeof(profile))==0 &&
        memcmp(&view,&view_before,sizeof(view))==0,
        "summary rendering is presentation-only"
    );

    Floppy144CompletionViewOpenFinalNote(&view);
    unresolved_final_hash=Draw(&view,&run,&profile,false,true);

    SetEvidence(&run,1U);
    Expect(
        Floppy144CompletionViewCoreResolutionEstablished(&run),
        "core-resolution evidence is detected from authored data"
    );
    Expect(
        Floppy144CompletionViewFinalConclusionText(&run)!=NULL &&
        strcmp(
            Floppy144CompletionViewFinalConclusionText(&run),
            final_conclusion
        )==0,
        "Final Note exposes the authored core-resolution conclusion verbatim"
    );

    final_hash=Draw(&view,&run,&profile,true,false);
    Expect(
        final_hash!=unresolved_final_hash,
        "resolved Final Note differs from unresolved capacity ending"
    );

    Floppy144CompletionViewScrollFinalNote(&view,&run,12);
    Expect(
        view.note_top_line>0U,
        "Final Note preserves scrollable completion evidence record"
    );

    Floppy144CompletionViewCloseFinalNote(&view);
    Expect(
        !Floppy144CompletionViewFinalNoteOpen(&view),
        "Final Note returns cleanly to completion summary"
    );

    if(failures!=0)
    {
        printf(
            "STAGE 4C BESPOKE COMPLETION TESTS: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4C BESPOKE COMPLETION TESTS: PASS\n");
    return 0;
}
