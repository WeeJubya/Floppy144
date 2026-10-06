/*
 * F144 Win32 logical-input adapter
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_win32_input.h"

F144Action f144Win32ActionFromVirtualKey(
    uint32_t virtual_key
)
{
    switch(virtual_key)
    {
        case VK_UP:
            return F144_ACTION_MOVE_UP;
        case VK_DOWN:
            return F144_ACTION_MOVE_DOWN;
        case VK_LEFT:
            return F144_ACTION_MOVE_LEFT;
        case VK_RIGHT:
            return F144_ACTION_MOVE_RIGHT;
        case 'W':
            return F144_ACTION_NAV_UP;
        case 'S':
            return F144_ACTION_NAV_DOWN;
        case 'A':
            return F144_ACTION_ACCESS;
        case 'I':
            return F144_ACTION_INSPECT;
        case VK_RETURN:
            return F144_ACTION_CONFIRM;
        case VK_BACK:
            return F144_ACTION_BACK;
        case VK_PRIOR:
            return F144_ACTION_PAGE_UP;
        case VK_NEXT:
            return F144_ACTION_PAGE_DOWN;
        case 'N':
            return F144_ACTION_NOTEBOOK;
        case VK_ESCAPE:
            return F144_ACTION_MENU;
        default:
            return F144_ACTION_NONE;
    }
}

void f144Win32TranslateKeyEvent(
    uint32_t virtual_key,
    F144ActionEventType type,
    F144ActionEvent *event
)
{
    if(event == NULL)
    {
        return;
    }

    event->action =
        f144Win32ActionFromVirtualKey(virtual_key);
    event->type =
        type;
    event->physical_token =
        virtual_key;
}

void f144Win32TranslateTextEvent(
    uint32_t codepoint,
    F144TextInputEvent *event
)
{
    if(event == NULL)
    {
        return;
    }

    event->codepoint =
        codepoint;
}
