/*
 * B2-04 native host test: platform API and native lifetime, with stubbed
 * save/audio/calendar services and no dependency on game or inherited runtime.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "floppy144_win32_host.h"
#include "f144_win32_timing.h"
#include "f144_win32_audio.h"
#include "f144_win32_calendar.h"
#include "f144_win32_storage.h"
#include <stdio.h>
#include <string.h>

static unsigned checks, failures, audio_inits, audio_stops, music_cue;
#define CHECK(ok,msg) do { ++checks; if(!(ok)) { ++failures; printf("FAIL: %s\n",msg); } } while(0)

bool f144Win32PlatformPersistencePath(F144Platform *p, F144PersistenceFile f, char *s, uint32_t n)
{ (void)p; if(f >= F144_PERSISTENCE_FILE_COUNT || !s || n < 2U) return false; s[0]='p';s[1]=0; return true; }
bool f144Win32PlatformLegacyPersistencePath(F144Platform *p, F144PersistenceFile f, uint32_t c, char *s, uint32_t n)
{ (void)c; return f144Win32PlatformPersistencePath(p,f,s,n); }
bool f144Win32AudioInit(F144Platform *p) { (void)p; ++audio_inits; return true; }
void f144Win32AudioShutdown(F144Platform *p) { (void)p; ++audio_stops; }
void f144Win32AudioPlayMusic(F144Platform *p,F144MusicCueId cue) { (void)p; music_cue=cue; }
void f144Win32AudioStopMusic(F144Platform *p) { (void)p; }
void f144Win32AudioPlaySfx(F144Platform *p,F144SfxCueId cue) { (void)p;(void)cue; }
void f144Win32AudioSetMusicVolume(F144Platform *p,uint8_t v) { (void)p;(void)v; }
void f144Win32AudioSetSfxVolume(F144Platform *p,uint8_t v) { (void)p;(void)v; }
void f144Win32AudioUpdate(F144Platform *p,uint64_t now,uint32_t r) { (void)p;(void)now;(void)r; }
bool f144Win32LocalCalendarDate(F144Platform *p,F144CalendarDate *d)
{ (void)p; if(!d) return false; d->year=2026U;d->month=10U;d->day=21U;return true; }

int main(void)
{
    F144Platform platform = {0};
    Floppy144Win32Host host = {0};
    Floppy144Surface *frame;
    HWND window;
    char path[F144_PLATFORM_PATH_CAPACITY] = {0};
    F144CalendarDate date = {0};

    CHECK(!Floppy144Win32HostBind(NULL,&host),"reject null platform");
    CHECK(!Floppy144Win32HostBind(&platform,NULL),"reject null host");
    CHECK(Floppy144Win32HostBind(&platform,&host),"bind native service table");
    CHECK(platform.api != NULL && platform.state == &host,"unchanged public opaque ABI");
    CHECK(f144PlatformFramebuffer(&platform) == NULL,"pre-init surface unavailable");
    CHECK(!Floppy144Win32HostStart(&platform,NULL),"reject null window");

    window = CreateWindowExA(0,"STATIC","B2-04 host",WS_OVERLAPPEDWINDOW,
                            0,0,800,600,NULL,NULL,GetModuleHandleA(NULL),NULL);
    CHECK(window != NULL,"native test window created");
    if(window == NULL) return 1;

    CHECK(Floppy144Win32HostStart(&platform,window),"framebuffer initialised");
    CHECK(!Floppy144Win32HostStart(&platform,window),"second start rejected");
    frame = f144PlatformFramebuffer(&platform);
    CHECK(frame != NULL && frame->width == 640U && frame->height == 360U &&
          frame->pixels != NULL,"borrowed 640x360 pixel surface");
    if(frame != NULL) frame->pixels[0] = 0x00FF0000U;
    CHECK(platform.surface.pixels == frame->pixels,"public surface is stable");

    CHECK(f144PlatformPersistencePath(&platform,F144_PERSISTENCE_PROFILE,path,
                                      (uint32_t)sizeof(path)) && path[0]=='p',
          "unchanged profile path slot");
    CHECK(f144PlatformLegacyPersistencePath(&platform,F144_PERSISTENCE_AUTOSAVE,
                                            0U,path,(uint32_t)sizeof(path)),
          "unchanged legacy persistence slot");
    CHECK(f144PlatformCalendarDate(&platform,NULL,&date) && date.year==2026U,
          "unchanged local calendar callback");
    CHECK(f144PlatformAudioInit(&platform),"audio callback initialised");
    CHECK(f144PlatformPlayMusic(&platform,5U) && music_cue==5U,
          "procedural cue forwarded unchanged");
    CHECK(f144PlatformSetMusicVolume(&platform,0U) &&
          f144PlatformSetSfxVolume(&platform,F144_AUDIO_VOLUME_MAX),
          "volume bounds retained");
    f144PlatformAudioUpdate(&platform,f144PlatformMonotonicMs(&platform),17U);
    f144PlatformAudioShutdown(&platform);
    CHECK(audio_inits==1U && audio_stops==1U,"audio exactly once");

    CHECK(f144Win32TimingStartWake(&platform,16U),"timer uses native HWND");
    CHECK(f144Win32TimingIsWakeMessage(WM_TIMER,1440U) &&
          !f144Win32TimingIsWakeMessage(WM_TIMER,1441U),"wake token");
    CHECK(!f144Win32TimingStartWake(&platform,0U),"zero interval fails");
    f144Win32TimingStopWake(&platform);

    f144PlatformQuit(&platform);
    CHECK(host.closing && !IsWindow(window),"quit destroys host window");
    Floppy144Win32HostWindowDestroyed(&platform,window);
    CHECK(Floppy144Win32HostWindow(&platform)==NULL,"stale native HWND cleared");
    f144PlatformQuit(&platform);
    Floppy144Win32HostStop(&platform);
    CHECK(f144PlatformFramebuffer(&platform)==NULL &&
          platform.surface.pixels==NULL,"shutdown releases public surface");
    Floppy144Win32HostStop(&platform);
    CHECK(host.display==NULL,"idempotent display cleanup");

    if(failures) { printf("B2-04 HOST TESTS: FAIL (%u/%u)\n",failures,checks); return 1; }
    printf("B2-04 HOST TESTS: PASS (%u assertions)\n",checks);
    return 0;
}
