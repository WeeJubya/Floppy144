/* BF38: headless Notebook page separation and navigation. */
#include "floppy144_notebook_view.h"
#include "floppy144_game_data.h"
#include <stdio.h>
#include <string.h>

#define TEST_DOCUMENTS 192U
static Floppy144DataRecord evidence[32], documents[TEST_DOCUMENTS];
static char evidence_text[32][64], document_text[TEST_DOCUMENTS][96];
static char captured[64][160];
static uint32_t shown, scroll_top, scroll_total, failures, footer_right;

static void Assert(bool ok,const char *msg) {
    if(!ok){++failures;printf("FAIL: %s\n",msg);}
}
static bool Has(const char *part) {
    uint32_t i;
    for(i=0U;i<shown;++i)if(strstr(captured[i],part))return true;
    return false;
}
/* Stub the unchanged BF18 ordered registry and pixel primitives; compile and
 * exercise the production Notebook view verbatim. */
const Floppy144DataRecord *Floppy144GameDataNotebookOrderedEntryAt(
    const Floppy144RunState *run,uint32_t i) {
    if(i<run->notebook_order_count)return i<32U?&evidence[i]:NULL;
    i-=run->notebook_order_count;
    return i<run->document_note_count&&i<TEST_DOCUMENTS?&documents[i]:NULL;
}
void Floppy144DrawClear(Floppy144Surface *s,uint32_t c){(void)s;(void)c;}
void Floppy144DrawFillRect(Floppy144Surface *s,uint32_t x,uint32_t y,
    uint32_t w,uint32_t h,uint32_t c) {
    (void)s;(void)x;(void)y;(void)w;(void)h;(void)c;
}
void Floppy144DrawRect(Floppy144Surface *s,uint32_t x,uint32_t y,
    uint32_t w,uint32_t h,uint32_t c) {
    (void)s;(void)x;(void)y;(void)w;(void)h;(void)c;
}
void Floppy144DrawScrollbar(Floppy144Surface *s,uint32_t x,uint32_t y,
    uint32_t h,uint32_t total,uint32_t visible,uint32_t top,
    uint32_t a,uint32_t b,uint32_t c) {
    (void)s;(void)x;(void)y;(void)h;(void)visible;
    (void)a;(void)b;(void)c;scroll_total=total;scroll_top=top;
}
uint32_t Floppy144DrawTextWidth(const char *s,uint32_t scale){
    return (uint32_t)strlen(s)*6U*scale;
}
void Floppy144DrawText(Floppy144Surface *s,uint32_t x,uint32_t y,
    const char *value,uint32_t scale,uint32_t colour) {
    (void)s;(void)colour;
    if(y==316U)footer_right=x+Floppy144DrawTextWidth(value,scale);
    if(shown<64U)(void)snprintf(captured[shown++],sizeof(captured[0]),"%s",value);
}
static void Draw(Floppy144NotebookViewState *view,Floppy144RunState *run) {
    static uint32_t pixel;
    Floppy144Surface s={&pixel,640U,360U};
    shown=0U;scroll_top=0U;scroll_total=0U;footer_right=0U;
    Floppy144NotebookViewDraw(&s,view,run);
}
int main(void) {
    Floppy144RunState run={0}, snapshot;
    Floppy144NotebookViewState view;
    uint32_t i;
    for(i=0U;i<32U;++i){
        (void)snprintf(evidence_text[i],sizeof(evidence_text[i]),
            "EVIDENCE %03u: A recovered finding.",(unsigned)i);
        evidence[i].pszA=evidence_text[i];
    }
    for(i=0U;i<TEST_DOCUMENTS;++i){
        (void)snprintf(document_text[i],sizeof(document_text[i]),
            "DR-RS-%04u: A first-open document summary.",(unsigned)i);
        documents[i].pszA=document_text[i];
    }
    Floppy144NotebookViewReset(&view);
    Draw(&view,&run);
    Assert(view.page==FLOPPY144_NOTEBOOK_PAGE_EVIDENCE&&
        Has("RECOVERY NOTEBOOK")&&Has("NO NOTES RECORDED."),
        "empty initial findings page");
    Assert(Has("RIGHT > Document Ledger")&&footer_right<=628U,
        "BF41 first-page label and footer fit");
    Floppy144NotebookViewSwitchPage(&view,1);
    Draw(&view,&run);
    Assert(Has("DOCUMENT LEDGER")&&Has("NO DOCUMENT SUMMARIES RECORDED."),
        "empty ledger");
    Assert(Has("< Recovery Notebook")&&footer_right<=628U,
        "BF41 ledger return label and footer fit");
    Floppy144NotebookViewSwitchPage(&view,1);
    Assert(view.page==FLOPPY144_NOTEBOOK_PAGE_DOCUMENT_LEDGER,
        "right boundary");
    Floppy144NotebookViewSwitchPage(&view,-1);
    Floppy144NotebookViewSwitchPage(&view,-1);
    Assert(view.page==FLOPPY144_NOTEBOOK_PAGE_EVIDENCE,"left boundary");
    run.notebook_order_count=30U;run.document_note_count=TEST_DOCUMENTS;
    snapshot=run;
    Draw(&view,&run);
    Assert(Has("EVIDENCE 000")&&!Has("DR-RS-")&&scroll_total==30U,
        "evidence separate from opened documents");
    Floppy144NotebookViewMove(&view,&run,10000);
    Draw(&view,&run);
    Assert(view.top_line==16U&&scroll_top==16U&&Has("EVIDENCE 029"),
        "evidence scroll clamping");
    Floppy144NotebookViewSwitchPage(&view,1);
    Draw(&view,&run);
    Assert(Has("DR-RS-0000")&&!Has("EVIDENCE 000")&&scroll_total==TEST_DOCUMENTS,
        "document ledger retains first-open order");
    Floppy144NotebookViewMove(&view,&run,10000);
    Draw(&view,&run);
    Assert(view.document_top_line==178U&&scroll_top==178U&&Has("DR-RS-0191"),
        "heavily populated ledger scrolls to end");
    Floppy144NotebookViewSwitchPage(&view,-1);
    Draw(&view,&run);
    Assert(scroll_top==16U&&Has("EVIDENCE 029"),"evidence scroll retained");
    Floppy144NotebookViewSwitchPage(&view,1);
    Draw(&view,&run);
    Assert(scroll_top==178U,"document scroll retained");
    Floppy144NotebookViewOpen(&view);
    Draw(&view,&run);
    Assert(view.page==FLOPPY144_NOTEBOOK_PAGE_EVIDENCE&&scroll_top==16U,
        "reopen defaults to first page");
    Assert(memcmp(&run,&snapshot,sizeof(run))==0,
        "navigation does not modify persisted gameplay state");
    Floppy144NotebookViewReset(&view);
    run.notebook_order_count=1U;run.document_note_count=1U;
    Floppy144NotebookViewSwitchPage(&view,1);
    Draw(&view,&run);
    Assert(Has("DR-RS-0000")&&!Has("EVIDENCE"),"first viewed document");
    ++run.document_note_count; /* First open in existing BF18 tracker. */
    Draw(&view,&run);
    Assert(Has("DR-RS-0001")&&Has("2 DOCUMENTS"),"newly opened document");
    Floppy144NotebookViewMove(&view,&run,-100);
    Assert(view.document_top_line==0U,"scroll clamps to zero");
    Floppy144NotebookViewReset(&view);
    Assert(view.page==FLOPPY144_NOTEBOOK_PAGE_EVIDENCE&&
        view.top_line==0U&&view.document_top_line==0U,"session reset");
    if(failures){printf("BF38: %u failures\n",(unsigned)failures);return 1;}
    puts("BF38 NOTEBOOK PAGINATION: PASS");return 0;
}
