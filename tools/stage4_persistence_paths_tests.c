/*
 * FLOPPY//144 Stage 4B persistence-path and migration regression.
 *
 * All paths live beneath F144_TEST_ROOT. The real user AppData directory is
 * never resolved or written by this test.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_platform.h"
#include "f144_win32_storage.h"

#include "floppy144_persistence.h"
#include "floppy144_profile.h"
#include "floppy144_run_state.h"
#include "floppy144_settings.h"
#include "floppy144_storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestPathContext
{
    char current_root[F144_PLATFORM_PATH_CAPACITY];
    char legacy_root[2][F144_PLATFORM_PATH_CAPACITY];
} TestPathContext;

static int failures;

static void Expect(bool condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

static const char *TestLeafName(F144PersistenceFile file)
{
    switch(file)
    {
        case F144_PERSISTENCE_MANUAL_SAVE: return "floppy144_manual.sav";
        case F144_PERSISTENCE_AUTOSAVE: return "floppy144_auto.sav";
        case F144_PERSISTENCE_PROFILE: return "floppy144_profile.dat";
        case F144_PERSISTENCE_SETTINGS: return "floppy144_settings.dat";
        default: return NULL;
    }
}

static bool TestJoinPath(
    const char *root,
    const char *leaf,
    char *path,
    uint32_t capacity
)
{
    int written;

    if(root==NULL||leaf==NULL||path==NULL||capacity==0U) return false;

    written=snprintf(path,capacity,"%s\\%s",root,leaf);
    if(written<0||(uint32_t)written>=capacity)
    {
        path[0]='\0';
        return false;
    }

    return true;
}

static bool TestMakeDirectory(const char *path)
{
    DWORD attributes;

    if(CreateDirectoryA(path,NULL)) return true;
    if(GetLastError()!=ERROR_ALREADY_EXISTS) return false;

    attributes=GetFileAttributesA(path);
    return
        attributes!=INVALID_FILE_ATTRIBUTES &&
        (attributes&FILE_ATTRIBUTE_DIRECTORY)!=0U;
}

static bool TestPrepareContext(
    const char *root,
    const char *name,
    TestPathContext *context
)
{
    char case_root[F144_PLATFORM_PATH_CAPACITY];

    if(root==NULL||name==NULL||context==NULL) return false;
    memset(context,0,sizeof(*context));

    if(
        !TestJoinPath(root,name,case_root,(uint32_t)sizeof(case_root)) ||
        !TestMakeDirectory(case_root) ||
        !TestJoinPath(
            case_root,
            "current-root",
            context->current_root,
            (uint32_t)sizeof(context->current_root)
        ) ||
        !TestJoinPath(
            case_root,
            "legacy-cwd",
            context->legacy_root[0],
            (uint32_t)sizeof(context->legacy_root[0])
        ) ||
        !TestJoinPath(
            case_root,
            "legacy-exe",
            context->legacy_root[1],
            (uint32_t)sizeof(context->legacy_root[1])
        )
    )
    {
        return false;
    }

    return
        TestMakeDirectory(context->current_root) &&
        TestMakeDirectory(context->legacy_root[0]) &&
        TestMakeDirectory(context->legacy_root[1]);
}

static Floppy144Surface *TestFramebuffer(F144Platform *platform)
{
    return &platform->surface;
}

static void TestPresent(F144Platform *platform)
{
    (void)platform;
}

static bool TestPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
)
{
    TestPathContext *context=(TestPathContext *)platform->state;

    return f144Win32PersistencePathForRoot(
        context->current_root,
        file,
        path,
        path_capacity
    );
}

static bool TestLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
)
{
    TestPathContext *context=(TestPathContext *)platform->state;
    const char *leaf=TestLeafName(file);

    if(candidate>=2U||leaf==NULL) return false;

    return TestJoinPath(
        context->legacy_root[candidate],
        leaf,
        path,
        path_capacity
    );
}

static const F144PlatformApi test_platform_api =
{
    TestFramebuffer,
    TestPresent,
    TestPersistencePath,
    TestLegacyPersistencePath
};

static void TestBindPlatform(F144Platform *platform,TestPathContext *context)
{
    memset(platform,0,sizeof(*platform));
    platform->api=&test_platform_api;
    platform->state=context;
}

static bool TestLegacyPath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path
)
{
    return f144PlatformLegacyPersistencePath(
        platform,
        file,
        candidate,
        path,
        F144_PLATFORM_PATH_CAPACITY
    );
}

static void TestCreateRun(Floppy144RunState *state,uint32_t seed)
{
    Floppy144RunStateBegin(state,seed);
}

static void TestWriteCorruptFile(const char *path)
{
    FILE *file=NULL;
    const char payload[]="not a FLOPPY//144 save";

    if(fopen_s(&file,path,"wb")==0&&file!=NULL)
    {
        (void)fwrite(payload,1U,sizeof(payload),file);
        (void)fclose(file);
    }
}

static void TestNoExistingData(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;

    Expect(TestPrepareContext(root,"no-data",&context),"no-data context");
    TestBindPlatform(&platform,&context);

    Expect(Floppy144StorageResolve(&platform,&paths),"no-data paths resolve");
    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "no-data migration is a no-op"
    );
    Expect(
        !Floppy144PersistenceFileExists(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE)
        ),
        "no-data does not manufacture a manual save"
    );
}

static void TestLegacyRunMigration(
    const char *root,
    F144PersistenceFile file,
    const char *case_name,
    uint32_t legacy_candidate,
    uint32_t seed
)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144RunState legacy_state;
    Floppy144RunState loaded_state;
    char legacy_path[F144_PLATFORM_PATH_CAPACITY];

    Expect(TestPrepareContext(root,case_name,&context),"legacy-run context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"legacy-run paths resolve");
    Expect(
        TestLegacyPath(
            &platform,
            file,
            legacy_candidate,
            legacy_path
        ),
        "legacy-run path resolves"
    );

    TestCreateRun(&legacy_state,seed);
    Expect(
        Floppy144PersistenceSaveRunState(legacy_path,&legacy_state),
        "legacy-run source saves"
    );
    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "legacy-run migration succeeds"
    );
    Expect(
        Floppy144PersistenceFileExists(legacy_path),
        "legacy-run source is retained"
    );

    memset(&loaded_state,0,sizeof(loaded_state));
    Expect(
        Floppy144PersistenceLoadRunState(
            Floppy144StoragePath(&paths,file),
            &loaded_state
        ),
        "migrated run reloads"
    );
    Expect(
        loaded_state.recovery_seed==seed,
        "migrated run preserves recovery seed"
    );
}

static void TestLegacyProfileMigration(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile loaded;
    char legacy_path[F144_PLATFORM_PATH_CAPACITY];

    Expect(TestPrepareContext(root,"legacy-profile",&context),"legacy-profile context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"legacy-profile paths resolve");
    Expect(
        TestLegacyPath(&platform,F144_PERSISTENCE_PROFILE,0U,legacy_path),
        "legacy-profile path resolves"
    );

    Floppy144DiscoveryProfileReset(&profile);
    profile.recovery_sessions_begun=17U;
    profile.dirty=1U;

    Expect(
        Floppy144PersistenceSaveProfile(legacy_path,&profile),
        "legacy profile saves"
    );
    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "legacy profile migration succeeds"
    );

    memset(&loaded,0,sizeof(loaded));
    Expect(
        Floppy144PersistenceLoadProfile(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_PROFILE),
            &loaded
        ),
        "migrated profile reloads"
    );
    Expect(
        loaded.recovery_sessions_begun==17U,
        "migrated profile preserves history"
    );
    Expect(
        Floppy144PersistenceFileExists(legacy_path),
        "legacy profile is retained"
    );
}

static void TestLegacySettingsMigration(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144Settings settings;
    Floppy144Settings loaded;
    char legacy_path[F144_PLATFORM_PATH_CAPACITY];

    Expect(TestPrepareContext(root,"legacy-settings",&context),"legacy-settings context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"legacy-settings paths resolve");
    Expect(
        TestLegacyPath(&platform,F144_PERSISTENCE_SETTINGS,0U,legacy_path),
        "legacy-settings path resolves"
    );

    Floppy144SettingsReset(&settings);
    (void)Floppy144SettingsSetMusicVolume(&settings,3U);

    Expect(
        Floppy144PersistenceSaveSettings(legacy_path,&settings),
        "legacy settings save"
    );
    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "legacy settings migration succeeds"
    );

    memset(&loaded,0,sizeof(loaded));
    Expect(
        Floppy144PersistenceLoadSettings(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_SETTINGS),
            &loaded
        ),
        "migrated settings reload"
    );
    Expect(loaded.music_volume==3U,"migrated settings preserve values");
    Expect(
        Floppy144PersistenceFileExists(legacy_path),
        "legacy settings are retained"
    );
}

static void TestNewLocationOnly(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144RunState current_state;
    Floppy144RunState loaded_state;

    Expect(TestPrepareContext(root,"new-only",&context),"new-only context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"new-only paths resolve");

    TestCreateRun(&current_state,777U);

    Expect(
        Floppy144PersistenceSaveRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &current_state
        ),
        "new-only current save"
    );

    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "new-only migration is a no-op"
    );

    memset(&loaded_state,0,sizeof(loaded_state));
    Expect(
        Floppy144PersistenceLoadRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &loaded_state
        ) &&
        loaded_state.recovery_seed==777U,
        "new-only current data remains intact"
    );
}

static void TestNewLocationWins(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144RunState current_state;
    Floppy144RunState legacy_state;
    Floppy144RunState loaded_state;
    char legacy_path[F144_PLATFORM_PATH_CAPACITY];

    Expect(TestPrepareContext(root,"new-wins",&context),"new-wins context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"new-wins paths resolve");
    Expect(
        TestLegacyPath(&platform,F144_PERSISTENCE_MANUAL_SAVE,0U,legacy_path),
        "new-wins legacy path resolves"
    );

    TestCreateRun(&current_state,900U);
    TestCreateRun(&legacy_state,100U);

    Expect(
        Floppy144PersistenceSaveRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &current_state
        ),
        "new-wins current save"
    );
    Expect(
        Floppy144PersistenceSaveRunState(legacy_path,&legacy_state),
        "new-wins legacy save"
    );
    Expect(
        Floppy144StorageMigrateLegacy(&platform,&paths)==0U,
        "new-wins migration does not overwrite"
    );

    memset(&loaded_state,0,sizeof(loaded_state));
    Expect(
        Floppy144PersistenceLoadRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &loaded_state
        ),
        "new-wins current reload"
    );
    Expect(
        loaded_state.recovery_seed==900U,
        "new AppData data wins when both files exist"
    );
}

static void TestRoundTrips(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    Floppy144RunState manual;
    Floppy144RunState autosave;
    Floppy144RunState loaded;
    Floppy144DiscoveryProfile profile;
    Floppy144DiscoveryProfile loaded_profile;
    Floppy144Settings settings;
    Floppy144Settings loaded_settings;

    Expect(TestPrepareContext(root,"round-trips",&context),"round-trip context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"round-trip paths resolve");

    TestCreateRun(&manual,301U);
    TestCreateRun(&autosave,302U);

    Expect(
        Floppy144PersistenceSaveRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &manual
        ),
        "manual save writes through resolved path"
    );
    Expect(
        Floppy144PersistenceSaveRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_AUTOSAVE),
            &autosave
        ),
        "autosave writes through resolved path"
    );

    memset(&loaded,0,sizeof(loaded));
    Expect(
        Floppy144PersistenceLoadRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE),
            &loaded
        ) &&
        loaded.recovery_seed==301U,
        "manual reinstate path reloads"
    );

    memset(&loaded,0,sizeof(loaded));
    Expect(
        Floppy144PersistenceLoadRunState(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_AUTOSAVE),
            &loaded
        ) &&
        loaded.recovery_seed==302U,
        "autosave reinstate path reloads"
    );

    Floppy144DiscoveryProfileReset(&profile);
    profile.recovery_sessions_begun=23U;
    profile.dirty=1U;

    Expect(
        Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_PROFILE),
            &profile
        ),
        "profile saves through resolved path"
    );

    memset(&loaded_profile,0,sizeof(loaded_profile));
    Expect(
        Floppy144PersistenceLoadProfile(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_PROFILE),
            &loaded_profile
        ) &&
        loaded_profile.recovery_sessions_begun==23U,
        "profile reloads through resolved path"
    );

    Floppy144SettingsReset(&settings);
    (void)Floppy144SettingsSetSfxVolume(&settings,4U);

    Expect(
        Floppy144PersistenceSaveSettings(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_SETTINGS),
            &settings
        ),
        "settings save through resolved path"
    );

    memset(&loaded_settings,0,sizeof(loaded_settings));
    Expect(
        Floppy144PersistenceLoadSettings(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_SETTINGS),
            &loaded_settings
        ) &&
        loaded_settings.sfx_volume==4U,
        "settings reload through resolved path"
    );
}

static void TestMalformedLegacy(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    char legacy_path[F144_PLATFORM_PATH_CAPACITY];
    uint32_t failures_mask;

    Expect(TestPrepareContext(root,"malformed-legacy",&context),"malformed context");
    TestBindPlatform(&platform,&context);
    Expect(Floppy144StorageResolve(&platform,&paths),"malformed paths resolve");
    Expect(
        TestLegacyPath(&platform,F144_PERSISTENCE_MANUAL_SAVE,0U,legacy_path),
        "malformed legacy path resolves"
    );

    TestWriteCorruptFile(legacy_path);
    failures_mask=Floppy144StorageMigrateLegacy(&platform,&paths);

    Expect(
        (
            failures_mask &
            (1U<<(uint32_t)F144_PERSISTENCE_MANUAL_SAVE)
        )!=0U,
        "malformed legacy save is reported"
    );
    Expect(
        !Floppy144PersistenceFileExists(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_MANUAL_SAVE)
        ),
        "malformed legacy save is not copied"
    );
    Expect(
        Floppy144PersistenceFileExists(legacy_path),
        "malformed legacy source is not deleted"
    );
}

static void TestMissingDirectoryAndCapacity(const char *root)
{
    TestPathContext context;
    F144Platform platform;
    Floppy144StoragePaths paths;
    char capacity_root[F144_PLATFORM_PATH_CAPACITY];
    char expected_directory[F144_PLATFORM_PATH_CAPACITY];
    char tiny[8]="x";
    DWORD attributes;

    memset(&context,0,sizeof(context));

    Expect(
        TestJoinPath(
            root,
            "missing-parent\\child",
            context.current_root,
            (uint32_t)sizeof(context.current_root)
        ),
        "missing-root string builds"
    );

    TestBindPlatform(&platform,&context);
    Expect(
        !Floppy144StorageResolve(&platform,&paths),
        "missing parent directory fails cleanly"
    );

    Expect(
        TestJoinPath(
            root,
            "capacity-root",
            capacity_root,
            (uint32_t)sizeof(capacity_root)
        ) &&
        TestMakeDirectory(capacity_root),
        "capacity root exists"
    );

    Expect(
        !f144Win32PersistencePathForRoot(
            capacity_root,
            F144_PERSISTENCE_MANUAL_SAVE,
            tiny,
            (uint32_t)sizeof(tiny)
        ) &&
        tiny[0]=='\0',
        "too-small path buffer fails without overflow"
    );

    Expect(
        TestJoinPath(
            capacity_root,
            "Floppy144",
            expected_directory,
            (uint32_t)sizeof(expected_directory)
        ),
        "expected data directory path builds"
    );

    attributes=GetFileAttributesA(expected_directory);
    Expect(
        attributes!=INVALID_FILE_ATTRIBUTES &&
        (attributes&FILE_ATTRIBUTE_DIRECTORY)!=0U,
        "Win32 path provider creates Floppy144 directory"
    );
}

int main(void)
{
    const char *root=getenv("F144_TEST_ROOT");

    if(root==NULL||root[0]=='\0')
    {
        printf("FAIL: F144_TEST_ROOT is not set\n");
        return 2;
    }

    TestNoExistingData(root);
    TestLegacyRunMigration(
        root,
        F144_PERSISTENCE_MANUAL_SAVE,
        "legacy-manual",
        0U,
        101U
    );
    TestLegacyRunMigration(
        root,
        F144_PERSISTENCE_AUTOSAVE,
        "legacy-autosave",
        1U,
        202U
    );
    TestLegacyProfileMigration(root);
    TestLegacySettingsMigration(root);
    TestNewLocationOnly(root);
    TestNewLocationWins(root);
    TestRoundTrips(root);
    TestMalformedLegacy(root);
    TestMissingDirectoryAndCapacity(root);

    if(failures!=0)
    {
        printf("STAGE 4 PERSISTENCE PATH TESTS: FAIL (%d)\n",failures);
        return 1;
    }

    printf("STAGE 4 PERSISTENCE PATH TESTS: PASS\n");
    return 0;
}
