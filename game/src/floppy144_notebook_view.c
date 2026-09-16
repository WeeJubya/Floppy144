/* Floppy//144 - data-driven player Notebook screen. */
#include "floppy144_notebook_view.h"
#include "floppy144_draw.h"
#include "floppy144_game_data.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define FLOPPY144_NOTEBOOK_BODY_LEFT 34U
#define FLOPPY144_NOTEBOOK_BODY_TOP 92U
#define FLOPPY144_NOTEBOOK_BODY_WIDTH 570U
#define FLOPPY144_NOTEBOOK_LINE_HEIGHT 15U
#define FLOPPY144_NOTEBOOK_MAX_LINES 12U

static void Floppy144NotebookDrawWrappedText(Floppy144Surface*pSurface,const char*pszText,uint32_t uColour)
{
    const char*pszCursor=pszText;char szLine[128];uint32_t uLine=0U,uLength=0U;
    if(!pSurface||!pszText)return;
    while(*pszCursor!='\0'&&uLine<FLOPPY144_NOTEBOOK_MAX_LINES)
    {
        const char*pszWordStart;uint32_t uWordLength,uCandidateLength;char szCandidate[128];
        if(*pszCursor=='\n'){if(uLength>0U){szLine[uLength]='\0';Floppy144DrawText(pSurface,FLOPPY144_NOTEBOOK_BODY_LEFT,FLOPPY144_NOTEBOOK_BODY_TOP+uLine*FLOPPY144_NOTEBOOK_LINE_HEIGHT,szLine,1U,uColour);++uLine;uLength=0U;}else ++uLine;++pszCursor;continue;}
        while(*pszCursor==' '||*pszCursor=='\t')++pszCursor;if(*pszCursor=='\0')break;
        pszWordStart=pszCursor;uWordLength=0U;while(pszCursor[uWordLength]!='\0'&&pszCursor[uWordLength]!=' '&&pszCursor[uWordLength]!='\t'&&pszCursor[uWordLength]!='\n')++uWordLength;
        if(uWordLength>=sizeof(szLine))uWordLength=(uint32_t)sizeof(szLine)-1U;
        uCandidateLength=uLength;if(uCandidateLength>0U&&uCandidateLength+1U<sizeof(szCandidate))szCandidate[uCandidateLength++]=' ';
        if(uCandidateLength+uWordLength>=sizeof(szCandidate))uWordLength=(uint32_t)sizeof(szCandidate)-uCandidateLength-1U;
        if(uLength>0U){uint32_t uCopy;for(uCopy=0U;uCopy<uLength;++uCopy)szCandidate[uCopy]=szLine[uCopy];}
        {uint32_t uCopy;for(uCopy=0U;uCopy<uWordLength;++uCopy)szCandidate[uCandidateLength+uCopy]=pszWordStart[uCopy];}
        uCandidateLength+=uWordLength;szCandidate[uCandidateLength]='\0';
        if(uLength>0U&&Floppy144DrawTextWidth(szCandidate,1U)>FLOPPY144_NOTEBOOK_BODY_WIDTH){szLine[uLength]='\0';Floppy144DrawText(pSurface,FLOPPY144_NOTEBOOK_BODY_LEFT,FLOPPY144_NOTEBOOK_BODY_TOP+uLine*FLOPPY144_NOTEBOOK_LINE_HEIGHT,szLine,1U,uColour);++uLine;uLength=0U;if(uLine>=FLOPPY144_NOTEBOOK_MAX_LINES)break;continue;}
        {uint32_t uCopy;for(uCopy=0U;uCopy<=uCandidateLength;++uCopy)szLine[uCopy]=szCandidate[uCopy];}
        uLength=uCandidateLength;pszCursor+=uWordLength;
    }
    if(uLength>0U&&uLine<FLOPPY144_NOTEBOOK_MAX_LINES){szLine[uLength]='\0';Floppy144DrawText(pSurface,FLOPPY144_NOTEBOOK_BODY_LEFT,FLOPPY144_NOTEBOOK_BODY_TOP+uLine*FLOPPY144_NOTEBOOK_LINE_HEIGHT,szLine,1U,uColour);}
}

void Floppy144NotebookViewReset(Floppy144NotebookViewState*pNotebook){if(pNotebook)pNotebook->selected_entry=0U;}
void Floppy144NotebookViewMove(Floppy144NotebookViewState*pNotebook,const Floppy144RunState*pRunState,int32_t nDirection)
{
    uint32_t uCount;int32_t nNext;if(!pNotebook||!pRunState||nDirection==0)return;uCount=Floppy144GameDataNotebookEntryCount(pRunState);if(uCount==0U){pNotebook->selected_entry=0U;return;}if(pNotebook->selected_entry>=uCount)pNotebook->selected_entry=uCount-1U;nNext=(int32_t)pNotebook->selected_entry+nDirection;while(nNext<0)nNext+=(int32_t)uCount;while(nNext>=(int32_t)uCount)nNext-=(int32_t)uCount;pNotebook->selected_entry=(uint32_t)nNext;
}

void Floppy144NotebookViewDraw(F144Runtime*pRuntime,const Floppy144NotebookViewState*pNotebook,const Floppy144RunState*pRunState)
{
    const uint32_t uBackground=FLOPPY144_RGB(16,15,13),uPanel=FLOPPY144_RGB(42,39,32),uPage=FLOPPY144_RGB(61,57,46),uBorder=FLOPPY144_RGB(112,103,80),uText=FLOPPY144_RGB(218,211,184),uMuted=FLOPPY144_RGB(143,135,111),uAmber=FLOPPY144_RGB(194,153,76);
    Floppy144Surface sSurface;uint32_t uCount,uSelected;const Floppy144DataRecord*pEntry;char szPosition[32],szTitle[96];
    if(!pRuntime||!pNotebook||!pRunState||!pRuntime->backbuffer.data)return;sSurface.pixels=(uint32_t*)pRuntime->backbuffer.data;sSurface.width=pRuntime->backbuffer.width;sSurface.height=pRuntime->backbuffer.height;
    uCount=Floppy144GameDataNotebookEntryCount(pRunState);uSelected=pNotebook->selected_entry;if(uCount>0U&&uSelected>=uCount)uSelected=uCount-1U;pEntry=uCount>0U?Floppy144GameDataNotebookEntryAt(pRunState,uSelected):NULL;
    Floppy144DrawClear(&sSurface,uBackground);Floppy144DrawText(&sSurface,10U,5U,"RECOVERY NOTEBOOK",1U,uMuted);if(uCount>0U)snprintf(szPosition,sizeof(szPosition),"ENTRY %02u OF %02u",(unsigned)(uSelected+1U),(unsigned)uCount);else snprintf(szPosition,sizeof(szPosition),"NO ENTRIES");Floppy144DrawText(&sSurface,520U,5U,szPosition,1U,uAmber);
    Floppy144DrawFillRect(&sSurface,10U,18U,610U,280U,uPanel);Floppy144DrawRect(&sSurface,10U,18U,610U,280U,uBorder);Floppy144DrawFillRect(&sSurface,24U,32U,582U,252U,uPage);Floppy144DrawRect(&sSurface,24U,32U,582U,252U,uBorder);
    if(!pEntry){Floppy144DrawText(&sSurface,34U,54U,"NO NOTES RECORDED.",1U,uAmber);Floppy144DrawText(&sSurface,34U,78U,"RECOVERED FACTS WILL APPEAR HERE AS THE SITE IS RECONSTRUCTED.",1U,uMuted);}else{const char*pszType="RECOVERED NOTE";if(pEntry->pszB&&strcmp(pEntry->pszB,"FACT")==0)pszType="RECOVERED FACT";else if(pEntry->pszB&&strcmp(pEntry->pszB,"EVIDENCE")==0)pszType="EVIDENCE";else if(pEntry->pszB&&strcmp(pEntry->pszB,"INTERACTION")==0)pszType="INSPECTION NOTE";snprintf(szTitle,sizeof(szTitle),"%s  %s",pEntry->pszId?pEntry->pszId:"NOTE",pEntry->pszC?pEntry->pszC:"RECOVERED FACT");Floppy144DrawText(&sSurface,34U,50U,szTitle,1U,uAmber);Floppy144DrawText(&sSurface,34U,66U,pszType,1U,uMuted);Floppy144DrawFillRect(&sSurface,34U,82U,562U,1U,uBorder);Floppy144NotebookDrawWrappedText(&sSurface,pEntry->pszA,uText);}
    Floppy144DrawFillRect(&sSurface,10U,306U,610U,28U,uBackground);Floppy144DrawRect(&sSurface,10U,306U,610U,28U,uBorder);Floppy144DrawText(&sSurface,22U,316U,"UP/DOWN ENTRY   PGUP/PGDN JUMP",1U,uText);Floppy144DrawText(&sSurface,476U,316U,"N/BACKSPACE RETURN",1U,uMuted);
}
