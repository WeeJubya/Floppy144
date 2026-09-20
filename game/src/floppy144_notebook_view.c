/* Floppy//144 - chronological continuous Notebook screen. */
#include "floppy144_notebook_view.h"
#include "floppy144_draw.h"
#include "floppy144_game_data.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define FLOPPY144_NOTEBOOK_BODY_LEFT 34U
#define FLOPPY144_NOTEBOOK_BODY_TOP 54U
#define FLOPPY144_NOTEBOOK_BODY_WIDTH 572U
#define FLOPPY144_NOTEBOOK_LINE_HEIGHT 15U
#define FLOPPY144_NOTEBOOK_VISIBLE_LINES 14U
#define FLOPPY144_NOTEBOOK_RENDER_LINES 512U
#define FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY 128U

typedef struct Floppy144NotebookRenderBuffer
{
    char lines[FLOPPY144_NOTEBOOK_RENDER_LINES][FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY];
    uint32_t count;
} Floppy144NotebookRenderBuffer;

static void Floppy144NotebookAppendWrapped(
    Floppy144NotebookRenderBuffer *pBuffer,
    const char *pszText
)
{
    const char *p=pszText;char line[FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY];uint32_t len=0U;bool first=true;
    if(pBuffer==NULL||pszText==NULL||pBuffer->count>=FLOPPY144_NOTEBOOK_RENDER_LINES)return;
    line[0]='*';line[1]=' ';len=2U;
    while(*p!='\0'&&pBuffer->count<FLOPPY144_NOTEBOOK_RENDER_LINES)
    {
        const char *word;uint32_t wordLen=0U,candidateLen;char candidate[FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY];
        while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')++p;if(*p=='\0')break;
        word=p;while(p[wordLen]!='\0'&&p[wordLen]!=' '&&p[wordLen]!='\t'&&p[wordLen]!='\n'&&p[wordLen]!='\r')++wordLen;
        candidateLen=len;if(candidateLen>2U&&candidateLen+1U<sizeof(candidate))candidate[candidateLen++]=' ';
        if(candidateLen+wordLen>=sizeof(candidate))wordLen=(uint32_t)sizeof(candidate)-candidateLen-1U;
        memcpy(candidate,line,len);memcpy(candidate+candidateLen,word,wordLen);candidateLen+=wordLen;candidate[candidateLen]='\0';
        if(len>2U&&Floppy144DrawTextWidth(candidate,1U)>FLOPPY144_NOTEBOOK_BODY_WIDTH)
        {
            line[len]='\0';(void)snprintf(pBuffer->lines[pBuffer->count++],FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY,"%s",line);
            line[0]=' ';line[1]=' ';len=2U;first=false;continue;
        }
        memcpy(line,candidate,candidateLen+1U);len=candidateLen;p+=wordLen;
    }
    if(len>2U&&pBuffer->count<FLOPPY144_NOTEBOOK_RENDER_LINES)
    {
        line[len]='\0';(void)snprintf(pBuffer->lines[pBuffer->count++],FLOPPY144_NOTEBOOK_RENDER_LINE_CAPACITY,"%s",line);
    }
    (void)first;
}

static void Floppy144NotebookBuild(
    const Floppy144RunState *pRunState,
    Floppy144NotebookRenderBuffer *pBuffer
)
{
    uint32_t u,count;
    if(pBuffer==NULL)return;pBuffer->count=0U;
    if(pRunState==NULL)return;
    count=Floppy144GameDataNotebookOrderedCount(pRunState);
    for(u=0U;u<count&&pBuffer->count<FLOPPY144_NOTEBOOK_RENDER_LINES;++u)
    {
        const Floppy144DataRecord *pEntry=Floppy144GameDataNotebookOrderedEntryAt(pRunState,u);
        if(pEntry!=NULL&&pEntry->pszA!=NULL)Floppy144NotebookAppendWrapped(pBuffer,pEntry->pszA);
    }
}

void Floppy144NotebookViewReset(Floppy144NotebookViewState *pNotebook)
{
    if(pNotebook!=NULL)pNotebook->top_line=0U;
}

void Floppy144NotebookViewMove(
    Floppy144NotebookViewState *pNotebook,
    const Floppy144RunState *pRunState,
    int32_t nDirection
)
{
    Floppy144NotebookRenderBuffer b;int32_t next,maxTop;
    if(pNotebook==NULL||pRunState==NULL||nDirection==0)return;
    Floppy144NotebookBuild(pRunState,&b);
    maxTop=b.count>FLOPPY144_NOTEBOOK_VISIBLE_LINES?(int32_t)(b.count-FLOPPY144_NOTEBOOK_VISIBLE_LINES):0;
    next=(int32_t)pNotebook->top_line+nDirection;if(next<0)next=0;if(next>maxTop)next=maxTop;pNotebook->top_line=(uint32_t)next;
}

void Floppy144NotebookViewDraw(
    F144Runtime *pRuntime,
    const Floppy144NotebookViewState *pNotebook,
    const Floppy144RunState *pRunState
)
{
    const uint32_t bg=FLOPPY144_RGB(16,15,13),panel=FLOPPY144_RGB(42,39,32),page=FLOPPY144_RGB(61,57,46),border=FLOPPY144_RGB(112,103,80),text=FLOPPY144_RGB(218,211,184),muted=FLOPPY144_RGB(143,135,111),amber=FLOPPY144_RGB(194,153,76);
    Floppy144Surface s;Floppy144NotebookRenderBuffer b;uint32_t top,u,count;char status[64];
    if(pRuntime==NULL||pNotebook==NULL||pRunState==NULL||pRuntime->backbuffer.data==NULL)return;
    s.pixels=(uint32_t*)pRuntime->backbuffer.data;s.width=pRuntime->backbuffer.width;s.height=pRuntime->backbuffer.height;
    Floppy144NotebookBuild(pRunState,&b);count=Floppy144GameDataNotebookOrderedCount(pRunState);top=pNotebook->top_line;
    if(b.count<=FLOPPY144_NOTEBOOK_VISIBLE_LINES)top=0U;else if(top>b.count-FLOPPY144_NOTEBOOK_VISIBLE_LINES)top=b.count-FLOPPY144_NOTEBOOK_VISIBLE_LINES;
    (void)snprintf(status,sizeof(status),"%u NOTES  LINE %u/%u",(unsigned)count,(unsigned)(b.count?top+1U:0U),(unsigned)b.count);
    Floppy144DrawClear(&s,bg);
    Floppy144DrawText(&s,10U,5U,"RECOVERY NOTEBOOK",1U,muted);
    Floppy144DrawText(&s,630U-Floppy144DrawTextWidth(status,1U),5U,status,1U,amber);
    Floppy144DrawFillRect(&s,10U,18U,620U,280U,panel);Floppy144DrawRect(&s,10U,18U,620U,280U,border);
    Floppy144DrawFillRect(&s,24U,32U,592U,252U,page);Floppy144DrawRect(&s,24U,32U,592U,252U,border);
    if(b.count==0U)
    {
        Floppy144DrawText(&s,34U,54U,"NO NOTES RECORDED.",1U,amber);
        Floppy144DrawText(&s,34U,78U,"RECOVERED FINDINGS WILL ACCUMULATE HERE.",1U,muted);
    }
    else
    {
        for(u=0U;u<FLOPPY144_NOTEBOOK_VISIBLE_LINES&&top+u<b.count;++u)
            Floppy144DrawText(&s,FLOPPY144_NOTEBOOK_BODY_LEFT,FLOPPY144_NOTEBOOK_BODY_TOP+u*FLOPPY144_NOTEBOOK_LINE_HEIGHT,b.lines[top+u],1U,text);
    }
    Floppy144DrawFillRect(&s,10U,306U,620U,28U,bg);Floppy144DrawRect(&s,10U,306U,620U,28U,border);
    Floppy144DrawText(&s,22U,316U,"UP/DOWN SCROLL   PGUP/PGDN PAGE",1U,text);
    Floppy144DrawText(&s,630U-Floppy144DrawTextWidth("N RETURN",1U)-12U,316U,"N RETURN",1U,muted);
}
