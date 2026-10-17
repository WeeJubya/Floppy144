#ifndef F144_SFX_H
#define F144_SFX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Compact sound-only IDs: 16 directly triggered effects. */
typedef enum F144SFX {
    F144_SFX_TERMINAL_KEY = 0,
    F144_SFX_TERMINAL_OK,
    F144_SFX_TERMINAL_ERROR,
    F144_SFX_RELAY,
    F144_SFX_FILING_CABINET,
    F144_SFX_CRT_WAKE,
    F144_SFX_POWER_FAIL,
    F144_SFX_FOOTSTEP,
    F144_SFX_FILE_RESTORE,
    F144_SFX_DOOR_BLOCKED,
    F144_SFX_EVIDENCE_FOUND,
    F144_SFX_DOOR_OPEN,
    F144_SFX_FLOPPY_INSERT,
    F144_SFX_TERMINAL_BACKSPACE,
    F144_SFX_RESTORE_TICK,
    F144_SFX_RESTORE_TICK_HIGH,
    F144_SFX_COUNT
} F144SFX;

int  F144_SFXInit(void);
void F144_SFXShutdown(void);

void F144_SFXPlay(F144SFX effect);

void  F144_SFXSetVolume(float volume);
float F144_SFXGetVolume(void);

int F144_SFXIsAvailable(void);
#ifdef F144_SFX_REVIEW
void F144_SFXReviewVariant(unsigned variant);
const char *F144_SFXReviewName(F144SFX effect);
#endif

#ifdef __cplusplus
}
#endif

#endif