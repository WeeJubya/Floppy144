/*
 * FLOPPY//144 bespoke completion presentation.
 *
 * Completion criteria and persistence live elsewhere. This module only reads
 * the frozen RunState/Profile and renders the achieved archival result.
 */
#include "floppy144_completion_view.h"

#include "floppy144_game_data.h"

#include <stdio.h>
#include <string.h>

#define FLOPPY144_COMPLETION_RENDER_LINES 256U
#define FLOPPY144_COMPLETION_LINE_CAPACITY 128U
#define FLOPPY144_COMPLETION_VISIBLE_LINES 12U
#define FLOPPY144_COMPLETION_BODY_WIDTH 520U

typedef struct Floppy144CompletionRenderBuffer
{
    char lines[FLOPPY144_COMPLETION_RENDER_LINES]
              [FLOPPY144_COMPLETION_LINE_CAPACITY];
    uint32_t count;
}
Floppy144CompletionRenderBuffer;

static void Floppy144CompletionTextCentred(
    Floppy144Surface *surface,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
)
{
    uint32_t width;

    if(surface==NULL||text==NULL) return;

    width=Floppy144DrawTextWidth(text,scale);
    Floppy144DrawText(
        surface,
        surface->width>width?(surface->width-width)/2U:0U,
        y,
        text,
        scale,
        colour
    );
}

static const Floppy144DataRecord *Floppy144CompletionCoreResolutionRecord(void)
{
    uint32_t index;

    for(index=0U;index<Floppy144GameDataRecordCount();++index)
    {
        const Floppy144DataRecord *record=
            Floppy144GameDataRecordAt(index);

        if(
            record!=NULL &&
            record->eKind==FLOPPY144_DATA_EVIDENCE &&
            record->pszD!=NULL &&
            strcmp(record->pszD,"Core resolution")==0
        )
        {
            return record;
        }
    }

    return NULL;
}

static const char *Floppy144CompletionEvidenceText(
    const Floppy144DataRecord *evidence
)
{
    if(evidence==NULL) return "";

    if(evidence->pszE!=NULL&&evidence->pszE[0]!='\0')
    {
        return evidence->pszE;
    }

    return evidence->pszA!=NULL?evidence->pszA:"";
}

static void Floppy144CompletionAppendLine(
    Floppy144CompletionRenderBuffer *buffer,
    const char *text
)
{
    if(
        buffer==NULL ||
        text==NULL ||
        buffer->count>=FLOPPY144_COMPLETION_RENDER_LINES
    )
    {
        return;
    }

    (void)snprintf(
        buffer->lines[buffer->count++],
        FLOPPY144_COMPLETION_LINE_CAPACITY,
        "%s",
        text
    );
}

static void Floppy144CompletionAppendWrapped(
    Floppy144CompletionRenderBuffer *buffer,
    const char *prefix,
    const char *text
)
{
    const char *cursor=text;
    char line[FLOPPY144_COMPLETION_LINE_CAPACITY];
    uint32_t length=0U;

    if(
        buffer==NULL ||
        text==NULL ||
        buffer->count>=FLOPPY144_COMPLETION_RENDER_LINES
    )
    {
        return;
    }

    if(prefix!=NULL)
    {
        length=(uint32_t)snprintf(line,sizeof(line),"%s",prefix);
        if(length>=sizeof(line)) length=(uint32_t)sizeof(line)-1U;
    }
    else
    {
        line[0]='\0';
    }

    while(
        *cursor!='\0' &&
        buffer->count<FLOPPY144_COMPLETION_RENDER_LINES
    )
    {
        const char *word;
        uint32_t word_length=0U;
        char candidate[FLOPPY144_COMPLETION_LINE_CAPACITY];
        uint32_t candidate_length;

        while(
            *cursor==' ' ||
            *cursor=='\t' ||
            *cursor=='\n' ||
            *cursor=='\r'
        )
        {
            ++cursor;
        }

        if(*cursor=='\0') break;

        word=cursor;

        while(
            word[word_length]!='\0' &&
            word[word_length]!=' ' &&
            word[word_length]!='\t' &&
            word[word_length]!='\n' &&
            word[word_length]!='\r'
        )
        {
            ++word_length;
        }

        candidate_length=length;
        memcpy(candidate,line,length);

        if(candidate_length>0U&&candidate_length+1U<sizeof(candidate))
        {
            candidate[candidate_length++]=' ';
        }

        if(candidate_length+word_length>=sizeof(candidate))
        {
            word_length=(uint32_t)sizeof(candidate)-candidate_length-1U;
        }

        memcpy(candidate+candidate_length,word,word_length);
        candidate_length+=word_length;
        candidate[candidate_length]='\0';

        if(
            length>0U &&
            Floppy144DrawTextWidth(candidate,1U)>
                FLOPPY144_COMPLETION_BODY_WIDTH
        )
        {
            line[length]='\0';
            Floppy144CompletionAppendLine(buffer,line);
            (void)snprintf(line,sizeof(line),"%s","       ");
            length=7U;
            continue;
        }

        memcpy(line,candidate,candidate_length+1U);
        length=candidate_length;
        cursor+=word_length;
    }

    if(
        length>0U &&
        buffer->count<FLOPPY144_COMPLETION_RENDER_LINES
    )
    {
        line[length]='\0';
        Floppy144CompletionAppendLine(buffer,line);
    }
}

static void Floppy144CompletionBuildFinalNote(
    const Floppy144RunState *run_state,
    Floppy144CompletionRenderBuffer *buffer
)
{
    uint32_t record_index;
    uint32_t evidence_number=0U;

    if(buffer==NULL) return;

    buffer->count=0U;

    if(run_state==NULL) return;


    if(
        Floppy144CompletionViewFinalConclusionText(
            run_state
        )!=NULL
    )
    {
        Floppy144CompletionAppendWrapped(
            buffer,
            "FINAL:",
            Floppy144CompletionViewFinalConclusionText(
                run_state
            )
        );
    }
    else
    {
        Floppy144CompletionAppendLine(
            buffer,
            "FINAL: [ CORE RESOLUTION NOT ESTABLISHED ]"
        );
    }

    Floppy144CompletionAppendLine(buffer,"");
    Floppy144CompletionAppendLine(buffer,"EVIDENCE RECORD");

    for(
        record_index=0U;
        record_index<Floppy144GameDataRecordCount();
        ++record_index
    )
    {
        const Floppy144DataRecord *evidence=
            Floppy144GameDataRecordAt(record_index);
        Floppy144EvidenceId evidence_id;
        char prefix[24];

        if(
            evidence==NULL ||
            evidence->eKind!=FLOPPY144_DATA_EVIDENCE ||
            evidence->pszId==NULL
        )
        {
            continue;
        }

        ++evidence_number;
        evidence_id=Floppy144GameDataEvidenceId(evidence->pszId);

        (void)snprintf(
            prefix,
            sizeof(prefix),
            "%u.",
            (unsigned)evidence_number
        );

        if(
            evidence_id!=FLOPPY144_EVIDENCE_COUNT &&
            Floppy144RunStateEvidenceEstablished(
                run_state,
                evidence_id
            )
        )
        {
            Floppy144CompletionAppendWrapped(
                buffer,
                prefix,
                Floppy144CompletionEvidenceText(evidence)
            );
        }
        else
        {
            Floppy144CompletionAppendWrapped(
                buffer,
                prefix,
                "[ DATA OBSCURED - EVIDENCE NOT RECOVERED ]"
            );
        }
    }
}

static uint32_t Floppy144CompletionFinalNoteMaxTop(
    const Floppy144RunState *run_state
)
{
    Floppy144CompletionRenderBuffer buffer;

    Floppy144CompletionBuildFinalNote(
        run_state,
        &buffer
    );

    return
        buffer.count>FLOPPY144_COMPLETION_VISIBLE_LINES
        ? buffer.count-FLOPPY144_COMPLETION_VISIBLE_LINES
        : 0U;
}

void Floppy144CompletionViewReset(
    Floppy144CompletionViewState *state
)
{
    if(state==NULL) return;

    memset(state,0,sizeof(*state));
    state->page=(uint8_t)FLOPPY144_COMPLETION_PAGE_SUMMARY;
    state->selected_option=
        (uint8_t)FLOPPY144_COMPLETION_OPTION_FINAL_NOTE;
}

void Floppy144CompletionViewMoveSelection(
    Floppy144CompletionViewState *state,
    int32_t direction
)
{
    int32_t next;

    if(
        state==NULL ||
        direction==0 ||
        state->page!=(uint8_t)FLOPPY144_COMPLETION_PAGE_SUMMARY
    )
    {
        return;
    }

    next=(int32_t)state->selected_option+
        (direction<0?-1:1);

    if(next<0)
    {
        next=(int32_t)FLOPPY144_COMPLETION_OPTION_COUNT-1;
    }
    else if(next>=(int32_t)FLOPPY144_COMPLETION_OPTION_COUNT)
    {
        next=0;
    }

    state->selected_option=(uint8_t)next;
}

Floppy144CompletionOption Floppy144CompletionViewSelectedOption(
    const Floppy144CompletionViewState *state
)
{
    if(
        state==NULL ||
        state->selected_option>=(uint8_t)FLOPPY144_COMPLETION_OPTION_COUNT
    )
    {
        return FLOPPY144_COMPLETION_OPTION_FINAL_NOTE;
    }

    return (Floppy144CompletionOption)state->selected_option;
}

bool Floppy144CompletionViewFinalNoteOpen(
    const Floppy144CompletionViewState *state
)
{
    return
        state!=NULL &&
        state->page==(uint8_t)FLOPPY144_COMPLETION_PAGE_FINAL_NOTE;
}

void Floppy144CompletionViewOpenFinalNote(
    Floppy144CompletionViewState *state
)
{
    if(state==NULL) return;

    state->page=(uint8_t)FLOPPY144_COMPLETION_PAGE_FINAL_NOTE;
    state->note_top_line=0U;
}

void Floppy144CompletionViewCloseFinalNote(
    Floppy144CompletionViewState *state
)
{
    if(state==NULL) return;

    state->page=(uint8_t)FLOPPY144_COMPLETION_PAGE_SUMMARY;
    state->note_top_line=0U;
}

void Floppy144CompletionViewScrollFinalNote(
    Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    int32_t lines
)
{
    uint32_t max_top;

    if(
        state==NULL ||
        state->page!=(uint8_t)FLOPPY144_COMPLETION_PAGE_FINAL_NOTE ||
        lines==0
    )
    {
        return;
    }

    max_top=Floppy144CompletionFinalNoteMaxTop(run_state);

    if(lines<0)
    {
        uint32_t amount=(uint32_t)(-lines);
        state->note_top_line=
            amount>state->note_top_line
            ? 0U
            : state->note_top_line-amount;
    }
    else
    {
        uint32_t amount=(uint32_t)lines;

        state->note_top_line=
            amount>max_top-state->note_top_line
            ? max_top
            : state->note_top_line+amount;
    }
}

uint32_t Floppy144CompletionViewCollectionsRestored(
    const Floppy144RunState *run_state
)
{
    uint32_t index;
    uint32_t count=0U;

    if(run_state==NULL) return 0U;

    for(index=0U;index<(uint32_t)FLOPPY144_COLLECTION_COUNT;++index)
    {
        if(
            Floppy144RunStateCollectionRestored(
                run_state,
                (Floppy144CollectionId)index
            )
        )
        {
            ++count;
        }
    }

    return count;
}

uint32_t Floppy144CompletionViewEvidenceEstablished(
    const Floppy144RunState *run_state
)
{
    uint32_t index;
    uint32_t count=0U;

    if(run_state==NULL) return 0U;

    for(index=0U;index<(uint32_t)FLOPPY144_EVIDENCE_COUNT;++index)
    {
        if(
            Floppy144RunStateEvidenceEstablished(
                run_state,
                (Floppy144EvidenceId)index
            )
        )
        {
            ++count;
        }
    }

    return count;
}

uint32_t Floppy144CompletionViewCapacityRemainingPercent(
    const Floppy144RunState *run_state
)
{
    uint32_t free_kb;

    if(run_state==NULL) return 0U;

    free_kb=Floppy144RunStateFreeKb(run_state);

    return
        FLOPPY144_RECOVERY_CAPACITY_KB==0U
        ? 0U
        : (
            free_kb*100U
        )/FLOPPY144_RECOVERY_CAPACITY_KB;
}

bool Floppy144CompletionViewCoreResolutionEstablished(
    const Floppy144RunState *run_state
)
{
    const Floppy144DataRecord *core=
        Floppy144CompletionCoreResolutionRecord();
    Floppy144EvidenceId evidence;

    if(
        run_state==NULL ||
        core==NULL ||
        core->pszId==NULL
    )
    {
        return false;
    }

    evidence=Floppy144GameDataEvidenceId(core->pszId);

    return
        evidence!=FLOPPY144_EVIDENCE_COUNT &&
        Floppy144RunStateEvidenceEstablished(
            run_state,
            evidence
        );
}

const char *Floppy144CompletionViewFinalConclusionText(
    const Floppy144RunState *run_state
)
{
    const Floppy144DataRecord *core=
        Floppy144CompletionCoreResolutionRecord();

    if(
        core==NULL ||
        !Floppy144CompletionViewCoreResolutionEstablished(
            run_state
        )
    )
    {
        return NULL;
    }

    return Floppy144CompletionEvidenceText(core);
}

const char *Floppy144CompletionViewOutcomeText(
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    if(evidence_resolved&&capacity_exhausted)
    {
        return
            "EVIDENCE RESOLVED / CAPACITY EXHAUSTED";
    }

    if(evidence_resolved)
    {
        return
            "EVIDENCE RESOLVED";
    }

    return
        "RECOVERY CAPACITY EXHAUSTED";
}

const char *Floppy144CompletionViewStatusText(
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    if(evidence_resolved&&capacity_exhausted)
    {
        return
            "CORE RESOLUTION ESTABLISHED AT THE RECOVERY LIMIT.";
    }

    if(evidence_resolved)
    {
        return
            "CORE RESOLUTION ESTABLISHED BEFORE CAPACITY EXHAUSTION.";
    }

    return
        "CAPACITY EXHAUSTED BEFORE CORE RESOLUTION WAS ESTABLISHED.";
}

static void Floppy144CompletionDrawSummary(
    Floppy144Surface *surface,
    const Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    const Floppy144DiscoveryProfile *profile,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    const uint32_t background=FLOPPY144_RGB(17,23,28);
    const uint32_t panel=FLOPPY144_RGB(24,33,39);
    const uint32_t panel_dark=FLOPPY144_RGB(12,17,21);
    const uint32_t border=FLOPPY144_RGB(86,103,107);
    const uint32_t text=FLOPPY144_RGB(202,211,205);
    const uint32_t muted=FLOPPY144_RGB(118,133,132);
    const uint32_t amber=FLOPPY144_RGB(194,153,76);
    const uint32_t green=FLOPPY144_RGB(100,156,111);
    static const char *const options[]=
    {
        "VIEW FINAL NOTE",
        "VIEW CREDITS",
        "RETURN TO MAIN MENU"
    };
    const char *operator_name=
        Floppy144DiscoveryProfileOperatorName(profile);
    char line[96];
    uint32_t collections=
        Floppy144CompletionViewCollectionsRestored(run_state);
    uint32_t evidence=
        Floppy144CompletionViewEvidenceEstablished(run_state);
    uint32_t free_kb=
        run_state!=NULL?Floppy144RunStateFreeKb(run_state):0U;
    uint32_t capacity_percent=
        Floppy144CompletionViewCapacityRemainingPercent(run_state);
    uint32_t option;

    if(operator_name==NULL||operator_name[0]=='\0')
    {
        operator_name="UNASSIGNED";
    }

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,0U,640U,16U,panel_dark);
    Floppy144DrawText(surface,10U,5U,"GDR RECOVERY COMPLETION RECORD",1U,muted);
    Floppy144DrawText(surface,556U,5U,"APS-12",1U,amber);

    Floppy144DrawFillRect(surface,24U,28U,592U,306U,panel);
    Floppy144DrawRect(surface,24U,28U,592U,306U,border);

    Floppy144CompletionTextCentred(
        surface,
        40U,
        "RECOVERY SESSION COMPLETE",
        2U,
        green
    );

    Floppy144CompletionTextCentred(
        surface,
        68U,
        Floppy144CompletionViewOutcomeText(
            evidence_resolved,
            capacity_exhausted
        ),
        1U,
        amber
    );

    Floppy144DrawFillRect(surface,48U,86U,544U,1U,border);

    (void)snprintf(
        line,
        sizeof(line),
        "OPERATOR: %s",
        operator_name
    );
    Floppy144DrawText(surface,56U,100U,line,1U,text);

    (void)snprintf(
        line,
        sizeof(line),
        "COLLECTIONS RESTORED:  %u / %u",
        (unsigned)collections,
        (unsigned)FLOPPY144_COLLECTION_COUNT
    );
    Floppy144DrawText(surface,56U,122U,line,1U,text);

    (void)snprintf(
        line,
        sizeof(line),
        "EVIDENCE ESTABLISHED: %u / %u",
        (unsigned)evidence,
        (unsigned)FLOPPY144_EVIDENCE_COUNT
    );
    Floppy144DrawText(surface,56U,140U,line,1U,text);

    (void)snprintf(
        line,
        sizeof(line),
        "CAPACITY REMAINING:   %u KB (%u%%)",
        (unsigned)free_kb,
        (unsigned)capacity_percent
    );
    Floppy144DrawText(surface,56U,158U,line,1U,text);

    Floppy144DrawFillRect(surface,48U,178U,544U,1U,border);
    Floppy144DrawText(surface,56U,190U,"FINAL ARCHIVAL STATUS",1U,amber);
    Floppy144DrawText(
        surface,
        56U,
        208U,
        Floppy144CompletionViewStatusText(
            evidence_resolved,
            capacity_exhausted
        ),
        1U,
        muted
    );

    Floppy144DrawFillRect(surface,48U,230U,544U,1U,border);

    for(option=0U;option<(uint32_t)FLOPPY144_COMPLETION_OPTION_COUNT;++option)
    {
        bool selected=
            state!=NULL &&
            state->selected_option==(uint8_t)option;
        uint32_t y=246U+option*21U;

        if(selected)
        {
            Floppy144DrawFillRect(surface,48U,y-4U,544U,16U,panel_dark);
            Floppy144DrawText(surface,86U,y,">",1U,amber);
        }

        Floppy144DrawText(
            surface,
            106U,
            y,
            options[option],
            1U,
            selected?amber:text
        );
    }

    Floppy144CompletionTextCentred(
        surface,
        FLOPPY144_UI_FORMAL_FOOTER_Y,
        "UP/DOWN SELECT   ENTER CONFIRM",
        1U,
        muted
    );
}

static void Floppy144CompletionDrawFinalNote(
    Floppy144Surface *surface,
    const Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    const uint32_t background=FLOPPY144_RGB(17,23,28);
    const uint32_t panel=FLOPPY144_RGB(24,33,39);
    const uint32_t panel_dark=FLOPPY144_RGB(12,17,21);
    const uint32_t border=FLOPPY144_RGB(86,103,107);
    const uint32_t text=FLOPPY144_RGB(202,211,205);
    const uint32_t muted=FLOPPY144_RGB(118,133,132);
    const uint32_t amber=FLOPPY144_RGB(194,153,76);
    const uint32_t green=FLOPPY144_RGB(100,156,111);
    Floppy144CompletionRenderBuffer buffer;
    uint32_t top;
    uint32_t max_top;
    uint32_t index;

    Floppy144CompletionBuildFinalNote(run_state,&buffer);

    max_top=
        buffer.count>FLOPPY144_COMPLETION_VISIBLE_LINES
        ? buffer.count-FLOPPY144_COMPLETION_VISIBLE_LINES
        : 0U;

    top=
        state!=NULL&&state->note_top_line<max_top
        ? state->note_top_line
        : max_top;

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,0U,640U,16U,panel_dark);
    Floppy144DrawText(surface,10U,5U,"GDR RECOVERY COMPLETION RECORD",1U,muted);
    Floppy144DrawText(surface,556U,5U,"APS-12",1U,amber);

    Floppy144DrawFillRect(surface,24U,28U,592U,306U,panel);
    Floppy144DrawRect(surface,24U,28U,592U,306U,border);

    Floppy144CompletionTextCentred(
        surface,
        40U,
        "FINAL ARCHIVAL NOTE",
        2U,
        green
    );

    Floppy144CompletionTextCentred(
        surface,
        68U,
        Floppy144CompletionViewOutcomeText(
            evidence_resolved,
            capacity_exhausted
        ),
        1U,
        amber
    );

    Floppy144DrawFillRect(surface,48U,88U,544U,1U,border);

    for(
        index=0U;
        index<FLOPPY144_COMPLETION_VISIBLE_LINES &&
        top+index<buffer.count;
        ++index
    )
    {
        Floppy144DrawText(
            surface,
            56U,
            102U+index*16U,
            buffer.lines[top+index],
            1U,
            text
        );
    }

    Floppy144DrawScrollbar(
        surface,
        586U,
        102U,
        188U,
        buffer.count,
        FLOPPY144_COMPLETION_VISIBLE_LINES,
        top,
        panel_dark,
        border,
        amber
    );

    Floppy144DrawFillRect(surface,48U,302U,544U,1U,border);
    Floppy144CompletionTextCentred(
        surface,
        FLOPPY144_UI_FORMAL_FOOTER_Y,
        "UP/DOWN SCROLL   PGUP/PGDN PAGE   ENTER/BACKSPACE BACK TO SUMMARY",
        1U,
        muted
    );
}

void Floppy144CompletionViewDraw(
    Floppy144Surface *surface,
    const Floppy144CompletionViewState *state,
    const Floppy144RunState *run_state,
    const Floppy144DiscoveryProfile *profile,
    bool evidence_resolved,
    bool capacity_exhausted
)
{
    if(surface==NULL||surface->pixels==NULL) return;

    if(Floppy144CompletionViewFinalNoteOpen(state))
    {
        Floppy144CompletionDrawFinalNote(
            surface,
            state,
            run_state,
            evidence_resolved,
            capacity_exhausted
        );
        return;
    }

    Floppy144CompletionDrawSummary(
        surface,
        state,
        run_state,
        profile,
        evidence_resolved,
        capacity_exhausted
    );
}
