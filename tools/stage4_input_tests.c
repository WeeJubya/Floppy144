/*
 * FLOPPY//144 Stage 4B logical-input regression
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_platform.h"
#include "f144_win32_input.h"
#include "floppy144_input.h"

#include <stdio.h>

static int failures;

_Static_assert(
    F144_ACTION_ACCESS != F144_ACTION_INSPECT,
    "Access and Inspect must remain distinct actions"
);

static void ExpectAction(
    uint32_t key,
    F144Action expected,
    const char *label
)
{
    F144Action actual =
        f144Win32ActionFromVirtualKey(key);

    if(actual != expected)
    {
        ++failures;
        printf("FAIL: %s expected %d got %d\n",label,(int)expected,(int)actual);
    }
}

static void ExpectEvent(
    uint32_t key,
    F144ActionEventType type,
    F144Action expected,
    const char *label
)
{
    F144ActionEvent event = {0};

    f144Win32TranslateKeyEvent(key,type,&event);

    if(
        event.action != expected ||
        event.type != type ||
        event.physical_token != key
    )
    {
        ++failures;
        printf("FAIL: %s event translation mismatch\n",label);
    }
}

int main(void)
{
    ExpectAction(VK_UP,F144_ACTION_MOVE_UP,"Up arrow");
    ExpectAction(VK_DOWN,F144_ACTION_MOVE_DOWN,"Down arrow");
    ExpectAction(VK_LEFT,F144_ACTION_MOVE_LEFT,"Left arrow");
    ExpectAction(VK_RIGHT,F144_ACTION_MOVE_RIGHT,"Right arrow");
    ExpectAction('W',F144_ACTION_NAV_UP,"W navigation alias");
    ExpectAction('S',F144_ACTION_NAV_DOWN,"S navigation alias");
    ExpectAction('A',F144_ACTION_ACCESS,"Access");
    ExpectAction('I',F144_ACTION_INSPECT,"Inspect");
    ExpectAction(VK_RETURN,F144_ACTION_CONFIRM,"Confirm");
    ExpectAction(VK_BACK,F144_ACTION_BACK,"Back");
    ExpectAction(VK_PRIOR,F144_ACTION_PAGE_UP,"Page Up");
    ExpectAction(VK_NEXT,F144_ACTION_PAGE_DOWN,"Page Down");
    ExpectAction('N',F144_ACTION_NOTEBOOK,"Notebook");
    ExpectAction(VK_ESCAPE,F144_ACTION_MENU,"Session menu");

    ExpectAction('Q',F144_ACTION_NONE,"Terminal Q remains text");
    ExpectAction('0',F144_ACTION_NONE,"Cabinet digit remains text");
    ExpectAction(VK_SPACE,F144_ACTION_NONE,"Terminal pager Space remains text");

    ExpectEvent(VK_F1,F144_ACTION_EVENT_DOWN,F144_ACTION_NONE,"Unmapped key down");
    ExpectEvent(VK_F1,F144_ACTION_EVENT_UP,F144_ACTION_NONE,"Unmapped key up");
    ExpectEvent(VK_RETURN,F144_ACTION_EVENT_DOWN,F144_ACTION_CONFIRM,"Mapped key down");
    ExpectEvent(VK_RETURN,F144_ACTION_EVENT_UP,F144_ACTION_CONFIRM,"Mapped key up");

    {
        Floppy144MovementInput movement = {0};
        int32_t x = 99;
        int32_t y = 99;

        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 0 || y != 0)
        {
            ++failures;
            printf("FAIL: reset movement vector is not stationary\n");
        }

        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_DOWN,
            true
        );
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 0 || y != 8)
        {
            ++failures;
            printf("FAIL: held Down does not move down\n");
        }

        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_LEFT,
            true
        );
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != -8 || y != 8)
        {
            ++failures;
            printf("FAIL: Down+Left does not resolve down-left\n");
        }

        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_LEFT,
            false
        );
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 0 || y != 8)
        {
            ++failures;
            printf("FAIL: releasing Left does not preserve held Down\n");
        }

        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_UP,
            true
        );
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 0 || y != 0)
        {
            ++failures;
            printf("FAIL: Up+Down should cancel vertically\n");
        }

        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_DOWN,
            false
        );
        (void)Floppy144MovementInputSetAction(
            &movement,
            F144_ACTION_MOVE_RIGHT,
            true
        );
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 8 || y != -8)
        {
            ++failures;
            printf("FAIL: Up+Right does not resolve up-right\n");
        }

        if(
            Floppy144MovementInputSetAction(
                &movement,
                F144_ACTION_NAV_UP,
                true
            )
        )
        {
            ++failures;
            printf("FAIL: navigation aliases must not become movement state\n");
        }

        Floppy144MovementInputReset(&movement);
        Floppy144MovementInputVector(&movement,8,&x,&y);
        if(x != 0 || y != 0)
        {
            ++failures;
            printf("FAIL: movement reset did not clear held directions\n");
        }
    }

    {
        F144TextInputEvent text_event = {0};

        f144Win32TranslateTextEvent((uint32_t)'Q',&text_event);
        if(text_event.codepoint != (uint32_t)'Q')
        {
            ++failures;
            printf("FAIL: terminal text event translation mismatch\n");
        }

        f144Win32TranslateTextEvent((uint32_t)'0',&text_event);
        if(text_event.codepoint != (uint32_t)'0')
        {
            ++failures;
            printf("FAIL: cabinet text event translation mismatch\n");
        }
    }

    if(failures != 0)
    {
        printf("STAGE 4 LOGICAL INPUT TESTS: FAIL (%d)\n",failures);
        return 1;
    }

    printf("STAGE 4 LOGICAL INPUT TESTS: PASS\n");
    return 0;
}
