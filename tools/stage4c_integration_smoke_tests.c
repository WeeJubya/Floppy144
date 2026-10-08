/*
 * FLOPPY//144 Stage 4C cross-surface integration smoke.
 *
 * This is a deterministic headless acceptance route through the same portable
 * Profile, Settings, Terminal, restored-session and Completion surfaces used
 * by the Windows application. Persistence round trips remain covered by the
 * dedicated Stage 4 persistence suite.
 */

#include "floppy144_completion_view.h"
#include "floppy144_credits_view.h"
#include "floppy144_game_data.h"
#include "floppy144_profile.h"
#include "floppy144_profile_edit.h"
#include "floppy144_profile_view.h"
#include "floppy144_recovery.h"
#include "floppy144_run_state.h"
#include "floppy144_settings.h"
#include "floppy144_settings_runtime.h"
#include "floppy144_settings_view.h"
#include "floppy144_terminal.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_PIXELS (640U*360U)

static int failures;
static uint32_t pixels[TEST_PIXELS];

static void Expect(bool condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

static bool TerminalContains(
    const Floppy144TerminalState *terminal,
    const char *needle
)
{
    uint32_t line;

    if(terminal==NULL||needle==NULL) return false;

    for(line=0U;line<terminal->output_count;++line)
    {
        if(strstr(terminal->output[line],needle)!=NULL) return true;
    }

    return false;
}

static void SubmitCommand(
    Floppy144TerminalState *terminal,
    Floppy144WorldState *world,
    Floppy144RunState *run_state,
    const char *command
)
{
    const char *cursor;

    for(cursor=command;cursor!=NULL&&*cursor!='\0';++cursor)
    {
        Floppy144TerminalInputCharacter(terminal,*cursor);
    }

    Floppy144TerminalSubmitInput(terminal,world,run_state);
}

static uint32_t HashPixels(void)
{
    uint32_t hash=2166136261U;
    uint32_t index;

    for(index=0U;index<TEST_PIXELS;++index)
    {
        hash^=pixels[index];
        hash*=16777619U;
    }

    return hash;
}

static uint32_t DrawProfile(
    const Floppy144DiscoveryProfile *profile,
    const Floppy144ProfileNameEditState *edit
)
{
    Floppy144Surface surface={pixels,640U,360U};
    memset(pixels,0,sizeof(pixels));
    Floppy144ProfileViewDraw(&surface,profile,edit);
    return HashPixels();
}

static uint32_t DrawSettings(
    const Floppy144Settings *settings
)
{
    Floppy144Surface surface={pixels,640U,360U};
    memset(pixels,0,sizeof(pixels));
    Floppy144SettingsViewDraw(
        &surface,
        settings,
        FLOPPY144_SETTINGS_OPTION_CRT,
        NULL
    );
    return HashPixels();
}

static uint32_t DrawRestored(
    const Floppy144RunState *recorded
)
{
    Floppy144Surface surface={pixels,640U,360U};
    Floppy144RunState empty;

    Floppy144RunStateReset(&empty);
    memset(pixels,0,sizeof(pixels));

    Floppy144MainMenuDraw(
        &surface,
        FLOPPY144_MAIN_MENU_REINSTATE_SESSION,
        false,
        true,
        &empty,
        recorded,
        NULL,
        NULL,
        false,
        true
    );

    return HashPixels();
}

static uint32_t DrawCredits(void)
{
    Floppy144Surface surface={pixels,640U,360U};
    memset(pixels,0,sizeof(pixels));
    Floppy144CreditsViewDraw(&surface,true);
    return HashPixels();
}

static uint32_t DrawCompletion(
    Floppy144CompletionViewState *view,
    const Floppy144RunState *run_state,
    const Floppy144DiscoveryProfile *profile,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    Floppy144Surface surface={pixels,640U,360U};
    memset(pixels,0,sizeof(pixels));

    Floppy144CompletionViewDraw(
        &surface,
        view,
        run_state,
        profile,
        evidence_resolved,
        capacity_exhausted
    );

    return HashPixels();
}

int main(void)
{
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile profile_before_view;
    Floppy144ProfileNameEditState edit;
    Floppy144Settings settings;
    Floppy144WorldState world;
    Floppy144RunState run_state;
    Floppy144RunState restored_run;
    Floppy144RunState completed_run;
    Floppy144TerminalState terminal;
    Floppy144CompletionViewState completion;
    Floppy144EvidenceId e020;
    Floppy144EvidenceId e021;
    Floppy144EvidenceId e022;
    uint32_t fresh_profile_hash;
    uint32_t named_profile_hash;
    uint32_t settings_hash;
    uint32_t restored_hash;
    uint32_t completion_hash;
    uint32_t final_note_hash;
    uint32_t credits_hash;
    const char *name="GLYNN WILLIAMS";
    const char *cursor;

    /* Fresh profile. */
    Floppy144DiscoveryProfileReset(&profile);
    Floppy144ProfileNameEditReset(&edit);

    Expect(
        !Floppy144DiscoveryProfileHasOperatorName(&profile) &&
        profile.recovery_sessions_begun==0U &&
        profile.completed_recoveries==0U,
        "fresh profile starts unnamed with zero recovery history"
    );

    fresh_profile_hash=DrawProfile(&profile,&edit);
    Expect(fresh_profile_hash!=0U,"fresh Profile surface renders");

    /* First-time identity entry. */
    Floppy144ProfileNameEditBegin(&edit,&profile,true);
    for(cursor=name;*cursor!='\0';++cursor)
    {
        Expect(
            Floppy144ProfileNameEditInputCodepoint(
                &edit,
                (uint32_t)(unsigned char)*cursor
            ),
            "operator name character accepted"
        );
    }

    Expect(
        Floppy144ProfileNameEditReadyToSave(&edit),
        "first-time operator name validates"
    );
    Expect(
        Floppy144DiscoveryProfileSetOperatorName(
            &profile,
            Floppy144ProfileNameEditText(&edit)
        ),
        "operator name commits to Profile scope"
    );
    Floppy144ProfileNameEditReset(&edit);

    /* Edit/cancel must not mutate the persistent identity. */
    Floppy144ProfileNameEditBegin(&edit,&profile,false);
    Expect(
        Floppy144ProfileNameEditInputCodepoint(&edit,(uint32_t)'X'),
        "name edit accepts a transient character"
    );
    Floppy144ProfileNameEditCancel(&edit);
    Expect(
        strcmp(
            Floppy144DiscoveryProfileOperatorName(&profile),
            name
        )==0,
        "cancelled name edit leaves persistent identity unchanged"
    );

    Expect(
        Floppy144DiscoveryProfileSetBodyStyle(
            &profile,
            FLOPPY144_OPERATOR_BODY_STYLE_B
        ) &&
        Floppy144DiscoveryProfileBodyStyle(&profile)==
            FLOPPY144_OPERATOR_BODY_STYLE_B,
        "body-style change remains in Profile scope"
    );

    named_profile_hash=DrawProfile(&profile,&edit);
    Expect(
        named_profile_hash!=0U &&
        named_profile_hash!=fresh_profile_hash,
        "named/body-style Profile presentation differs from fresh profile"
    );

    /* Every visible Settings control has a live Core-side value. */
    Floppy144SettingsReset(&settings);
    Expect(
        Floppy144SettingsSetCrtMode(&settings,FLOPPY144_CRT_REDUCED),
        "CRT setting changes"
    );
    Expect(
        Floppy144SettingsSetTextSpeed(&settings,FLOPPY144_TEXT_SPEED_FAST),
        "text-speed setting changes"
    );
    Expect(
        Floppy144SettingsSetMusicVolume(&settings,6U),
        "music-volume setting changes"
    );
    Expect(
        Floppy144SettingsSetSfxVolume(&settings,7U),
        "SFX-volume setting changes"
    );
    Expect(
        Floppy144SettingsSetAutosaveMode(
            &settings,
            FLOPPY144_AUTOSAVE_10_MINUTES
        ),
        "autosave setting changes"
    );
    Expect(
        Floppy144SettingsTextElapsedMs(&settings,100U)==200U &&
        Floppy144SettingsAutosaveIntervalMs(&settings)==600000U,
        "text speed and autosave expose their runtime values"
    );

    settings_hash=DrawSettings(&settings);
    Expect(settings_hash!=0U,"Settings surface renders");

    /* Begin one fresh recovery and authenticate exactly once. */
    Floppy144DiscoveryProfileBeginRecovery(&profile);
    Floppy144WorldReset(&world);
    Floppy144RunStateBegin(&run_state,144U);
    Floppy144TerminalReset(&terminal,&world);
    Floppy144TerminalConfigureSession(&terminal,false,true,true);
    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        Floppy144DiscoveryProfileOperatorName(&profile),
        true
    );

    Expect(
        TerminalContains(&terminal,"OPERATOR: GLYNN WILLIAMS") &&
        TerminalContains(&terminal,"VERIFYING PROFILE...") &&
        TerminalContains(&terminal,"ACCESS ACCEPTED"),
        "fresh recovery authenticates the named operator"
    );

    /* Exercise terminal text entry, commands, LIST pager and history. */
    SubmitCommand(&terminal,&world,&run_state,"INITIATE");
    Expect(
        Floppy144WorldArchiveServicesInitialised(&world) &&
        Floppy144RunStateArchiveServicesInitialised(&run_state),
        "INITIATE still drives the established Stage 3 terminal path"
    );

    SubmitCommand(&terminal,&world,&run_state,"LIST");
    Expect(
        terminal.history_count>=2U &&
        strcmp(
            terminal.history[terminal.history_count-1U],
            "LIST"
        )==0 &&
        terminal.output_count>0U,
        "LIST remains accepted by the fresh-session terminal path"
    );
    if(Floppy144TerminalRecordPagerActive(&terminal))
    {
        Floppy144TerminalCloseRecordPager(&terminal);
    }

    for(cursor="RESTORE D";*cursor!='\0';++cursor)
    {
        Floppy144TerminalInputCharacter(&terminal,*cursor);
    }
    Floppy144TerminalMoveHistory(&terminal,-1);
    Expect(
        strcmp(terminal.input,"LIST")==0,
        "history recalls the newest command"
    );
    Floppy144TerminalMoveHistory(&terminal,1);
    Expect(
        strcmp(terminal.input,"RESTORE D")==0,
        "history restores the unfinished edit draft"
    );
    Floppy144TerminalBackspace(&terminal);
    Expect(
        strcmp(terminal.input,"RESTORE ")==0,
        "terminal text editing still supports backspace"
    );

    /* Repeated physical access keeps identity without replaying auth. */
    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_MAIN_OFFICE
    );
    Floppy144TerminalConfigureSession(&terminal,false,true,true);
    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        Floppy144DiscoveryProfileOperatorName(&profile),
        false
    );
    Expect(
        TerminalContains(&terminal,"OPERATOR: GLYNN WILLIAMS") &&
        !TerminalContains(&terminal,"VERIFYING PROFILE..."),
        "repeat terminal access does not replay authentication"
    );

    /* Headless restored-session route. Reinstate does not begin a new recovery. */
    restored_run=run_state;
    restored_hash=DrawRestored(&restored_run);
    Expect(restored_hash!=0U,"SESSION RESTORED presentation renders");

    Floppy144TerminalResetAtRoom(
        &terminal,
        &world,
        FLOPPY144_ROOM_RECORDS_OFFICE
    );
    Floppy144TerminalConfigureSession(&terminal,false,true,true);
    Floppy144TerminalApplyOperatorIdentity(
        &terminal,
        Floppy144DiscoveryProfileOperatorName(&profile),
        true
    );
    Expect(
        TerminalContains(&terminal,"VERIFYING PROFILE...") &&
        profile.recovery_sessions_begun==1U,
        "restored application session authenticates once without incrementing recovery history"
    );

    /*
     * Complete the authored E-020/E-021/E-022 synthesis path rather than
     * manufacturing an alternative completion rule.
     */
    Floppy144RunStateBegin(&completed_run,145U);
    e020=Floppy144GameDataEvidenceId("E-020");
    e021=Floppy144GameDataEvidenceId("E-021");
    e022=Floppy144GameDataEvidenceId("E-022");

    Expect(
        e020!=FLOPPY144_EVIDENCE_COUNT &&
        e021!=FLOPPY144_EVIDENCE_COUNT &&
        e022!=FLOPPY144_EVIDENCE_COUNT,
        "authored completion evidence IDs resolve"
    );

    (void)Floppy144RunStateEstablishEvidence(&completed_run,e020);
    (void)Floppy144RunStateEstablishEvidence(&completed_run,e021);
    (void)Floppy144RunStateEstablishEvidence(&completed_run,e022);
    Floppy144GameDataResolveEvidence(&completed_run);

    Expect(
        Floppy144GameDataEvidenceResolved(&completed_run) &&
        Floppy144RunStateAct(&completed_run)==FLOPPY144_RUN_ACT_COMPLETE,
        "fresh-profile smoke reaches the established authored completion state"
    );

    (void)Floppy144DiscoveryProfileMergeRunState(
        &profile,
        &completed_run
    );
    Expect(
        Floppy144DiscoveryProfileRecordCompletion(
            &profile,
            &completed_run,
            true,
            false
        ),
        "completion snapshot records once"
    );
    Expect(
        profile.completed_recoveries==1U &&
        (
            profile.latest_completion_flags &
            FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED
        )!=0U,
        "completion history contains the achieved ending"
    );

    profile_before_view=profile;
    Floppy144CompletionViewReset(&completion);
    completion_hash=DrawCompletion(
        &completion,
        &completed_run,
        &profile,
        true,
        false
    );
    Floppy144CompletionViewOpenFinalNote(&completion);
    final_note_hash=DrawCompletion(
        &completion,
        &completed_run,
        &profile,
        true,
        false
    );
    credits_hash=DrawCredits();

    Expect(
        completion_hash!=0U &&
        final_note_hash!=0U &&
        credits_hash!=0U &&
        completion_hash!=final_note_hash,
        "Completion summary, Final Note and Credits all render in the same accepted route"
    );
    Expect(
        memcmp(&profile,&profile_before_view,sizeof(profile))==0,
        "Completion/Credits viewing cannot increment or rewrite Profile history"
    );

    /* A subsequent new recovery remains valid and does not duplicate completion. */
    Floppy144DiscoveryProfileBeginRecovery(&profile);
    Expect(
        profile.recovery_sessions_begun==2U &&
        profile.completed_recoveries==1U,
        "subsequent recovery increments session history but not completion history"
    );

    if(failures!=0)
    {
        printf(
            "STAGE 4C INTEGRATION SMOKE: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4C INTEGRATION SMOKE: PASS\n");
    return 0;
}
