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
#include "floppy144_document.h"
#include "floppy144_game_data.h"
#include "floppy144_collection_registry.h"
#include "floppy144_catalogue.h"
#include "floppy144_variation.h"
#include "floppy144_grey_door.h"
#include "floppy144_site_object.h"
#include "floppy144_grey_encounter.h"
#include "floppy144_site_rooms.h"
#include "floppy144_profile.h"
#include "floppy144_profile_view.h"
#include "floppy144_run_state.h"
#include "floppy144_settings.h"
#include "floppy144_storage.h"

#include <ctype.h>
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

/*
 * Populate every persistent discovery-profile field used by the player-facing
 * Profile screen so migration/round-trip tests exercise the real history.
 */
static void TestPopulateProfileHistory(
    Floppy144DiscoveryProfile *profile,
    const char *operator_name,
    uint32_t sessions
)
{
    if(
        profile == NULL ||
        operator_name == NULL
    )
    {
        return;
    }

    Floppy144DiscoveryProfileReset(
        profile
    );

    (void)Floppy144DiscoveryProfileSetOperatorName(
        profile,
        operator_name
    );

    (void)Floppy144DiscoveryProfileSetBodyStyle(
        profile,
        FLOPPY144_OPERATOR_BODY_STYLE_B
    );

    profile->recovery_sessions_begun =
        sessions;

    (void)Floppy144DiscoveryProfileRecordCollection(
        profile,
        (Floppy144CollectionId)0
    );

    (void)Floppy144DiscoveryProfileRecordCollection(
        profile,
        (Floppy144CollectionId)(
            FLOPPY144_COLLECTION_COUNT -
            1
        )
    );

    (void)Floppy144DiscoveryProfileRecordEvidence(
        profile,
        (Floppy144EvidenceId)0
    );

    (void)Floppy144DiscoveryProfileRecordEvidence(
        profile,
        (Floppy144EvidenceId)(
            FLOPPY144_EVIDENCE_COUNT -
            1
        )
    );

    profile->latest_completion_evidence[0] =
        1U;

    profile->completed_recoveries =
        2U;

    profile->latest_completion_evidence_percent =
        78U;

    profile->latest_completion_flags =
        FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED;

    profile->latest_completion_recovered_kb =
        1234U;

    profile->dirty =
        1U;
}

/*
 * Verify a loaded profile retained every field displayed by Profile.
 */
static bool TestProfileHistoryMatches(
    const Floppy144DiscoveryProfile *profile,
    const char *operator_name,
    uint32_t sessions
)
{
    if(
        profile == NULL ||
        operator_name == NULL
    )
    {
        return false;
    }

    return
        strcmp(
            profile->operator_name,
            operator_name
        ) == 0 &&
        profile->body_style ==
            (uint8_t)FLOPPY144_OPERATOR_BODY_STYLE_B &&
        profile->recovery_sessions_begun ==
            sessions &&
        Floppy144DiscoveryProfileCollectionsEverRestoredCount(
            profile
        ) == 2U &&
        Floppy144DiscoveryProfileEvidenceEverEstablishedCount(
            profile
        ) == 2U &&
        profile->latest_completion_evidence[0] ==
            1U &&
        profile->completed_recoveries ==
            2U &&
        profile->latest_completion_evidence_percent ==
            78U &&
        profile->latest_completion_flags ==
            FLOPPY144_PROFILE_COMPLETION_EVIDENCE_RESOLVED &&
        profile->latest_completion_recovered_kb ==
            1234U;
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

    TestPopulateProfileHistory(
        &profile,
        "LEGACY OPERATOR",
        17U
    );

    /*
     * Pre-S4C-02 files may contain a NUL-terminated name which the new player
     * editor would not offer. Loading/migration remains schema-compatible and
     * must preserve that historical value rather than rejecting the profile.
     */
    (void)snprintf(
        profile.operator_name,
        sizeof(profile.operator_name),
        "%s",
        "LEGACY@OPERATOR"
    );

    profile.dirty =
        1U;

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
        TestProfileHistoryMatches(
            &loaded,
            "LEGACY@OPERATOR",
            17U
        ),
        "migrated profile preserves complete legacy operator history"
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

/*
 * Render the persistent profile after reload so body-style persistence is
 * covered through the existing player-facing presentation path.
 */
static uint32_t TestProfileRenderHash(
    const Floppy144DiscoveryProfile *profile
)
{
    enum
    {
        TEST_PROFILE_WIDTH = 640,
        TEST_PROFILE_HEIGHT = 360
    };

    Floppy144Surface surface;
    uint32_t *pixels;
    uint32_t hash;
    uint32_t index;
    uint32_t count;

    if(profile == NULL)
    {
        return 0U;
    }

    count =
        (uint32_t)(
            TEST_PROFILE_WIDTH *
            TEST_PROFILE_HEIGHT
        );

    pixels =
        (uint32_t *)malloc(
            (size_t)count *
            sizeof(uint32_t)
        );

    if(pixels == NULL)
    {
        return 0U;
    }

    memset(
        pixels,
        0,
        (size_t)count *
        sizeof(uint32_t)
    );

    surface.pixels = pixels;
    surface.width = TEST_PROFILE_WIDTH;
    surface.height = TEST_PROFILE_HEIGHT;

    Floppy144ProfileViewDraw(
        &surface,
        profile,
        NULL
    );

    hash = 2166136261U;

    for(index = 0U; index < count; ++index)
    {
        hash ^= pixels[index];
        hash *= 16777619U;
    }

    free(pixels);
    return hash;
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
    Floppy144DiscoveryProfile decoded_profile;
    Floppy144Settings settings;
    Floppy144Settings loaded_settings;
    Floppy144Settings decoded_settings;
    uint8_t settings_payload[FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE];
    uint8_t profile_payload[
        FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
    ];
    uint32_t type_a_hash;
    uint32_t type_b_hash;
    uint32_t style_index;

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

    TestPopulateProfileHistory(
        &profile,
        "Glynn Williams",
        23U
    );

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
        TestProfileHistoryMatches(
            &loaded_profile,
            "Glynn Williams",
            23U
        ),
        "profile history reloads through resolved path"
    );

    /*
     * Profile identity is independent of a recovery save. Beginning another
     * recovery increments profile history but must preserve the same name.
     * Saving and reloading again models an application restart.
     */
    Floppy144DiscoveryProfileBeginRecovery(
        &loaded_profile
    );

    Expect(
        strcmp(
            loaded_profile.operator_name,
            "Glynn Williams"
        ) == 0,
        "starting another recovery preserves operator identity"
    );

    Expect(
        Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_PROFILE),
            &loaded_profile
        ),
        "updated profile persists after another recovery begins"
    );

    memset(
        &profile,
        0,
        sizeof(profile)
    );

    Expect(
        Floppy144PersistenceLoadProfile(
            Floppy144StoragePath(&paths,F144_PERSISTENCE_PROFILE),
            &profile
        ) &&
        TestProfileHistoryMatches(
            &profile,
            "Glynn Williams",
            24U
        ),
        "operator name survives restart after another recovery"
    );

    /*
     * Body style remains a profile-only cosmetic value. Exercise both styles,
     * repeated changes, restart persistence, legacy zero/default semantics,
     * malformed-value normalisation, and presentation after reload.
     */
    type_b_hash =
        TestProfileRenderHash(
            &profile
        );

    Expect(
        type_b_hash != 0U,
        "reloaded Type B profile renders"
    );

    Expect(
        Floppy144DiscoveryProfileSetBodyStyle(
            &profile,
            FLOPPY144_OPERATOR_BODY_STYLE_A
        ),
        "body style changes to Type A"
    );

    Expect(
        Floppy144PersistenceSaveProfile(
            Floppy144StoragePath(
                &paths,
                F144_PERSISTENCE_PROFILE
            ),
            &profile
        ),
        "Type A body style saves"
    );

    memset(&loaded_profile,0,sizeof(loaded_profile));

    Expect(
        Floppy144PersistenceLoadProfile(
            Floppy144StoragePath(
                &paths,
                F144_PERSISTENCE_PROFILE
            ),
            &loaded_profile
        ) &&
        Floppy144DiscoveryProfileBodyStyle(
            &loaded_profile
        ) == FLOPPY144_OPERATOR_BODY_STYLE_A,
        "Type A body style survives restart"
    );

    type_a_hash =
        TestProfileRenderHash(
            &loaded_profile
        );

    Expect(
        type_a_hash != 0U &&
        type_a_hash != type_b_hash,
        "reloaded styles produce distinct profile previews"
    );

    for(style_index = 0U; style_index < 8U; ++style_index)
    {
        Floppy144OperatorBodyStyle expected =
            (style_index & 1U) == 0U
                ? FLOPPY144_OPERATOR_BODY_STYLE_B
                : FLOPPY144_OPERATOR_BODY_STYLE_A;

        Expect(
            Floppy144DiscoveryProfileSetBodyStyle(
                &loaded_profile,
                expected
            ),
            "body style can be changed repeatedly before save"
        );

        Expect(
            Floppy144PersistenceSaveProfile(
                Floppy144StoragePath(
                    &paths,
                    F144_PERSISTENCE_PROFILE
                ),
                &loaded_profile
            ),
            "repeated body-style change persists"
        );

        memset(&profile,0,sizeof(profile));

        Expect(
            Floppy144PersistenceLoadProfile(
                Floppy144StoragePath(
                    &paths,
                    F144_PERSISTENCE_PROFILE
                ),
                &profile
            ) &&
            Floppy144DiscoveryProfileBodyStyle(
                &profile
            ) == expected,
            "repeated body-style change survives reload"
        );

        loaded_profile = profile;
    }

    Expect(
        Floppy144PersistenceEncodeProfile(
            &loaded_profile,
            profile_payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        ),
        "body-style compatibility profile encodes"
    );

    profile_payload[
        FLOPPY144_PROFILE_NAME_CAPACITY
    ] = 0U;

    memset(&decoded_profile,0,sizeof(decoded_profile));

    Expect(
        Floppy144PersistenceDecodeProfile(
            &decoded_profile,
            profile_payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        ) &&
        Floppy144DiscoveryProfileBodyStyle(
            &decoded_profile
        ) == FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT &&
        decoded_profile.dirty == 0U,
        "old/uninitialised zero body style remains valid default Type A"
    );

    profile_payload[
        FLOPPY144_PROFILE_NAME_CAPACITY
    ] = 0xFEU;

    memset(&decoded_profile,0,sizeof(decoded_profile));

    Expect(
        Floppy144PersistenceDecodeProfile(
            &decoded_profile,
            profile_payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        ) &&
        Floppy144DiscoveryProfileBodyStyle(
            &decoded_profile
        ) == FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT &&
        decoded_profile.dirty != 0U,
        "invalid body style normalises safely without rejecting profile"
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

    Expect(
        Floppy144PersistenceEncodeSettings(
            &loaded_settings,
            settings_payload,
            FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
        ),
        "settings compatibility payload encodes"
    );

    settings_payload[0]=0xffU;
    settings_payload[1]=0xffU;
    settings_payload[2]=0xffU;
    settings_payload[3]=0xffU;
    settings_payload[4]=0xffU;
    memset(&decoded_settings,0,sizeof(decoded_settings));

    Expect(
        Floppy144PersistenceDecodeSettings(
            &decoded_settings,
            settings_payload,
            FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
        ) &&
        decoded_settings.crt_mode==(uint8_t)FLOPPY144_SETTINGS_DEFAULT_CRT_MODE &&
        decoded_settings.text_speed==(uint8_t)FLOPPY144_SETTINGS_DEFAULT_TEXT_SPEED &&
        decoded_settings.music_volume==FLOPPY144_SETTINGS_DEFAULT_MUSIC_VOLUME &&
        decoded_settings.sfx_volume==FLOPPY144_SETTINGS_DEFAULT_SFX_VOLUME &&
        decoded_settings.autosave_mode==(uint8_t)FLOPPY144_SETTINGS_DEFAULT_AUTOSAVE_MODE &&
        decoded_settings.dirty!=0U,
        "out-of-range V1 settings normalise field-by-field"
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

/*
 * Verify that Stage 3's historical working-directory probe follows the actual
 * launch CWD while the executable-directory compatibility probe remains tied
 * to the executable location. This models launching Stage 4 from a different
 * working directory without touching the user's real AppData directory.
 */
static void TestDifferentWorkingDirectory(
    const char *root
)
{
    char original_cwd[F144_PLATFORM_PATH_CAPACITY];
    char alternate_cwd[F144_PLATFORM_PATH_CAPACITY];
    char executable_path[F144_PLATFORM_PATH_CAPACITY];
    char executable_directory[F144_PLATFORM_PATH_CAPACITY];
    char expected_cwd_legacy[F144_PLATFORM_PATH_CAPACITY];
    char expected_exe_legacy[F144_PLATFORM_PATH_CAPACITY];
    char actual_cwd_legacy[F144_PLATFORM_PATH_CAPACITY];
    char actual_exe_legacy[F144_PLATFORM_PATH_CAPACITY];
    char current_before[F144_PLATFORM_PATH_CAPACITY];
    char current_after[F144_PLATFORM_PATH_CAPACITY];
    char *separator;
    DWORD length;
    bool changed;

    memset(
        original_cwd,
        0,
        sizeof(original_cwd)
    );

    memset(
        alternate_cwd,
        0,
        sizeof(alternate_cwd)
    );

    memset(
        executable_path,
        0,
        sizeof(executable_path)
    );

    memset(
        executable_directory,
        0,
        sizeof(executable_directory)
    );

    memset(
        expected_cwd_legacy,
        0,
        sizeof(expected_cwd_legacy)
    );

    memset(
        expected_exe_legacy,
        0,
        sizeof(expected_exe_legacy)
    );

    memset(
        actual_cwd_legacy,
        0,
        sizeof(actual_cwd_legacy)
    );

    memset(
        actual_exe_legacy,
        0,
        sizeof(actual_exe_legacy)
    );

    memset(
        current_before,
        0,
        sizeof(current_before)
    );

    memset(
        current_after,
        0,
        sizeof(current_after)
    );

    length =
        GetCurrentDirectoryA(
            (DWORD)sizeof(original_cwd),
            original_cwd
        );

    Expect(
        length > 0U &&
        length < (DWORD)sizeof(original_cwd),
        "original working directory resolves"
    );

    Expect(
        TestJoinPath(
            root,
            "alternate-cwd",
            alternate_cwd,
            (uint32_t)sizeof(alternate_cwd)
        ) &&
        TestMakeDirectory(
            alternate_cwd
        ),
        "alternate working directory exists"
    );

    length =
        GetModuleFileNameA(
            NULL,
            executable_path,
            (DWORD)sizeof(executable_path)
        );

    Expect(
        length > 0U &&
        length < (DWORD)sizeof(executable_path),
        "test executable path resolves"
    );

    (void)snprintf(
        executable_directory,
        sizeof(executable_directory),
        "%s",
        executable_path
    );

    separator =
        strrchr(
            executable_directory,
            '\\'
        );

    if(separator == NULL)
    {
        separator =
            strrchr(
                executable_directory,
                '/'
            );
    }

    Expect(
        separator != NULL,
        "test executable directory resolves"
    );

    if(separator != NULL)
    {
        *separator =
            '\0';
    }

    Expect(
        f144Win32PersistencePathForRoot(
            root,
            F144_PERSISTENCE_MANUAL_SAVE,
            current_before,
            (uint32_t)sizeof(current_before)
        ),
        "current persistence path resolves before CWD change"
    );

    changed =
        SetCurrentDirectoryA(
            alternate_cwd
        ) != 0;

    Expect(
        changed,
        "process working directory changes for launch simulation"
    );

    if(changed)
    {
        Expect(
            TestJoinPath(
                alternate_cwd,
                "floppy144_manual.sav",
                expected_cwd_legacy,
                (uint32_t)sizeof(expected_cwd_legacy)
            ),
            "expected CWD legacy path builds"
        );

        Expect(
            TestJoinPath(
                executable_directory,
                "floppy144_manual.sav",
                expected_exe_legacy,
                (uint32_t)sizeof(expected_exe_legacy)
            ),
            "expected executable legacy path builds"
        );

        Expect(
            f144Win32PlatformLegacyPersistencePath(
                NULL,
                F144_PERSISTENCE_MANUAL_SAVE,
                0U,
                actual_cwd_legacy,
                (uint32_t)sizeof(actual_cwd_legacy)
            ),
            "legacy CWD candidate resolves after working-directory change"
        );

        Expect(
            _stricmp(
                actual_cwd_legacy,
                expected_cwd_legacy
            ) == 0,
            "legacy CWD candidate follows the launch working directory"
        );

        Expect(
            f144Win32PlatformLegacyPersistencePath(
                NULL,
                F144_PERSISTENCE_MANUAL_SAVE,
                1U,
                actual_exe_legacy,
                (uint32_t)sizeof(actual_exe_legacy)
            ),
            "legacy executable-directory candidate resolves"
        );

        Expect(
            _stricmp(
                actual_exe_legacy,
                expected_exe_legacy
            ) == 0,
            "legacy executable-directory candidate ignores launch CWD"
        );

        Expect(
            f144Win32PersistencePathForRoot(
                root,
                F144_PERSISTENCE_MANUAL_SAVE,
                current_after,
                (uint32_t)sizeof(current_after)
            ) &&
            _stricmp(
                current_before,
                current_after
            ) == 0,
            "current persistence location is independent of launch CWD"
        );
    }

    Expect(
        SetCurrentDirectoryA(
            original_cwd
        ) != 0,
        "original working directory is restored"
    );
}


/* S4G-01: isolated orphan record, stable placement, V1/V2/V3 compatibility. */
static void TestGreyDoorDiscovery(const char *root)
{
    Floppy144RunState run = {0}, before = {0}, loaded = {0}, older = {0};
    Floppy144DiscoveryProfile profile, profile_before;
    Floppy144CollectionId collection = FLOPPY144_COLLECTION_DR01;
    uint32_t index = 999U;
    uint32_t kb, slot;
    uint8_t old_v2[FLOPPY144_SAVE_PAYLOAD_V2_SIZE];
    char path[F144_PLATFORM_PATH_CAPACITY];
    char id[24], title[48];

    Floppy144RunStateBegin(&run,146U);
    Floppy144DiscoveryProfileReset(&profile);
    profile_before=profile;

    Expect(
        Floppy144DocumentFindRecordId(
            FLOPPY144_GREY_DOOR_RECORD_ID,&collection,&index
        ) &&
        collection == FLOPPY144_GREY_DOOR_RECORD_COLLECTION &&
        index == FLOPPY144_GREY_DOOR_RECORD_INDEX,
        "S4G orphan record resolves outside all normal collections"
    );
    Expect(!Floppy144DocumentAccessible(&run,collection,index),
        "S4G orphan unavailable until DR-01 is restored");
    Expect(!Floppy144DocumentApplyEffects(NULL,&run,collection,index),
        "S4G cannot be opened before recovery");
    (void)Floppy144RunStateBitSet(
        run.collections,(uint32_t)FLOPPY144_COLLECTION_DR01
    );
    Floppy144CatalogueBuildRecordForSeed(
        collection,index,run.recovery_seed,id,sizeof(id),title,sizeof(title)
    );
    Expect(strcmp(id,FLOPPY144_GREY_DOOR_RECORD_ID)==0 &&
        strcmp(title,"Unallocated Floor Area Notice")==0,
        "S4G orphan uses ordinary authored catalogue title and record ID");
    Expect(Floppy144DocumentAccessible(&run,collection,index),
        "S4G orphan visible once DR-01 exists");
    before=run;
    kb=Floppy144RunStateRecoveredKb(&run);
    slot=Floppy144RunStateGreyDoorPlacementSlot(&run,12U);
    Expect(Floppy144DocumentApplyEffects(NULL,&run,collection,index) &&
        run.grey_door_state == (uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
        run.dirty == 1U,
        "S4G first successful view enables availability and save dirtiness");
    Expect(Floppy144RunStateRecoveredKb(&run)==kb &&
        memcmp(run.collections,before.collections,sizeof(run.collections))==0 &&
        memcmp(run.triggers,before.triggers,sizeof(run.triggers))==0 &&
        memcmp(run.evidence,before.evidence,sizeof(run.evidence))==0 &&
        memcmp(run.notebook,before.notebook,sizeof(run.notebook))==0 &&
        memcmp(run.notebook_order,before.notebook_order,
            sizeof(run.notebook_order))==0 &&
        run.notebook_order_count==before.notebook_order_count &&
        memcmp(&profile,&profile_before,sizeof(profile))==0,
        "S4G costs no capacity and changes no gameplay or profile counters");
    run.dirty=0U;
    (void)Floppy144DocumentApplyEffects(NULL,&run,collection,index);
    (void)Floppy144VariationValue(146U,"staff.crossword.scribble.v1","P-074");
    Expect(run.grey_door_state == (uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
        run.dirty==0U &&
        Floppy144RunStateGreyDoorPlacementSlot(&run,12U)==slot,
        "S4G repeated view neither dirties save nor rerolls placement");
    Expect(TestJoinPath(root,"grey-door.sav",path,(uint32_t)sizeof(path)),
        "S4G isolated save path");
    Expect(Floppy144PersistenceSaveRunState(path,&run) &&
        Floppy144PersistenceLoadRunState(path,&loaded),
        "S4G V3 save/reload works");
    Expect(loaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
        loaded.recovery_seed==run.recovery_seed &&
        Floppy144RunStateGreyDoorPlacementSlot(&loaded,12U)==slot,
        "S4G reload preserves availability and derived placement");
    Expect(Floppy144RunStateGreyDoorComplete(&loaded) &&
        loaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED,
        "S4G completion is a single persistent transition");
    loaded.dirty=0U;
    (void)Floppy144DocumentApplyEffects(NULL,&loaded,collection,index);
    Expect(loaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
        loaded.dirty==0U &&
        !Floppy144RunStateGreyDoorComplete(&loaded) &&
        !Floppy144RunStateGreyDoorDiscover(&loaded),
        "S4G completed door cannot be revived by document or second completion");
    Expect(Floppy144PersistenceSaveRunState(path,&loaded) &&
        Floppy144PersistenceLoadRunState(path,&run) &&
        run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED,
        "S4G permanent completion survives save/reload");
    Expect(Floppy144PersistenceEncodeRunState(
        &run,old_v2,FLOPPY144_SAVE_PAYLOAD_V2_SIZE) &&
        Floppy144PersistenceDecodeRunState(
            &older,old_v2,FLOPPY144_SAVE_PAYLOAD_V2_SIZE) &&
        older.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE,
        "S4G legacy V2 load defaults the optional door to unavailable");
}

/*
 * Enumerate EVERY candidate, not just lucky seeds. With unchanged Site data,
 * every exposed wall slot must remain clear, solid, physically reachable and
 * within the established half-unit logical action distance.
 */
static void TestGreyDoorPlacement(const char *root)
{
    static const uint32_t seeds[] = {
        0U,1U,42U,144U,146U,2026U,65535U,0x12345678U,0xffffffffU
    };
    Floppy144RunState run, reloaded;
    Floppy144GreyDoorCandidate candidate;
    uint32_t count=Floppy144GreyDoorCandidateCount();
    uint32_t i, first_slot=0U;
    bool seed_varied=false;
    char save_path[F144_PLATFORM_PATH_CAPACITY];
    uint32_t original_rect_count=Floppy144SiteRectCount();

    Expect(count>=2U,
        "S4G safe candidate inventory is nonempty and permits seed variation");
    for(i=0U;i<count;++i)
    {
        uint32_t j;
        Expect(Floppy144GreyDoorCandidateAt(i,&candidate),
            "S4G every candidate can be resolved");
        Expect(Floppy144GreyDoorCandidateSafe(&candidate),
            "S4G each candidate revalidates against ALL generated geometry");
        Expect(Floppy144SiteRoomAtPosition(
                candidate.stand_x16,candidate.stand_y16)==
                FLOPPY144_ROOM_CORRIDOR &&
            !Floppy144SitePositionBlocked(
                candidate.stand_x16,candidate.stand_y16),
            "S4G each candidate has a collision-free corridor interaction position");
        for(j=0U;j<Floppy144SiteRectCount();++j)
        {
            const Floppy144SiteRect *existing=Floppy144SiteRectAt(j);
            const Floppy144SiteRect *door=&candidate.rect;
            if(existing==NULL ||
               existing->type<=(uint8_t)FLOPPY144_SITE_FLOOR_D) continue;
            Expect(!(
                (int32_t)existing->x < (int32_t)door->x+door->width+2 &&
                (int32_t)existing->x+existing->width > (int32_t)door->x-2 &&
                (int32_t)existing->y < (int32_t)door->y+door->height+2 &&
                (int32_t)existing->y+existing->height > (int32_t)door->y-2
            ), "S4G no door/window/noticeboard/directory/fixture intersects 2U halo");
        }
    }

    Expect(TestJoinPath(root,"grey-door-placement.sav",save_path,
        (uint32_t)sizeof(save_path)),"S4G placement test save path");
    for(i=0U;i<(uint32_t)(sizeof(seeds)/sizeof(seeds[0]));++i)
    {
        uint32_t slot, original_kb;
        Floppy144RunStateBegin(&run,seeds[i]);
        (void)Floppy144RunStateBitSet(
            run.rooms,(uint32_t)FLOPPY144_ROOM_CORRIDOR);
        run.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE;
        slot=Floppy144RunStateGreyDoorPlacementSlot(&run,count);
        if(i==0U) first_slot=slot;
        else if(slot!=first_slot) seed_varied=true;
        original_kb=Floppy144RunStateRecoveredKb(&run);
        Expect(Floppy144GreyDoorForRun(&run,&candidate) &&
            Floppy144GreyDoorCandidateSafe(&candidate),
            "S4G seeded selection always chooses a vetted candidate");
        Floppy144RunStateSetPlayerSitePosition(
            &run,candidate.stand_x16,candidate.stand_y16);
        Expect(Floppy144GreyDoorNearby(&run),
            "S4G ordinary 0.5U inspect/access proximity works");
        (void)Floppy144VariationValue(run.recovery_seed,
            "staff.crossword.scribble.v1","P-074");
        Expect(Floppy144RunStateGreyDoorPlacementSlot(&run,count)==slot,
            "S4G dedicated seed stream cannot be changed by other features");
        Expect(Floppy144PersistenceSaveRunState(save_path,&run) &&
            Floppy144PersistenceLoadRunState(save_path,&reloaded),
            "S4G selected Door state survives real save/reload");
        Expect(Floppy144GreyDoorForRun(&reloaded,&candidate) &&
            Floppy144GreyDoorNearby(&reloaded) &&
            Floppy144RunStateGreyDoorPlacementSlot(&reloaded,count)==slot,
            "S4G save/reload preserves selected wall and interaction stance");
        Expect(Floppy144RunStateGreyDoorComplete(&reloaded) &&
            !Floppy144GreyDoorForRun(&reloaded,&candidate) &&
            !Floppy144GreyDoorNearby(&reloaded) &&
            Floppy144RunStateRecoveredKb(&reloaded)==original_kb,
            "S4G completion instantly removes overlay and action without capacity change");
        Expect(Floppy144PersistenceSaveRunState(save_path,&reloaded) &&
            Floppy144PersistenceLoadRunState(save_path,&run) &&
            !Floppy144GreyDoorForRun(&run,&candidate) &&
            !Floppy144GreyDoorNearby(&run),
            "S4G wall returns to normal after completed-run restart");
        Expect(Floppy144SiteRectCount()==original_rect_count,
            "S4G rendering/interaction never alters authoritative Site geometry");
    }
    Expect(seed_varied,"S4G at least two fixed seeds select different safe walls");
    printf("S4G-02 corridor candidate audit: %u valid placements, %u seeds\n",
        (unsigned)count,(unsigned)(sizeof(seeds)/sizeof(seeds[0])));
}


/* BF35 focused timing and accelerated 144% contract. */
static void TestBF35EncounterTiming(void)
{
    Floppy144GreyEncounter scene={0},manual={0};
    uint32_t k,early,middle,late;
    scene.phase=(uint8_t)FLOPPY144_GREY_EXPLORE;
    scene.restoration_start=27U;
    scene.local_x=115;
    scene.local_y=264;
    Floppy144PlayerVisualReset(&scene.player_visual);
    for(k=0U;k<49U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    Expect(scene.phase==(uint8_t)FLOPPY144_GREY_EXPLORE &&
        scene.elapsed_ms==4900U &&
        Floppy144GreyEncounterDisplayPercent(&scene)==27U,
        "BF35 no auto-turn before five seconds");
    (void)Floppy144GreyEncounterAdvance(&scene,100U);
    Expect(scene.phase==(uint8_t)FLOPPY144_GREY_TURN &&
        !scene.developer_inspected,
        "BF35 automatic turn exactly after five seconds");
    for(k=0U;k<19U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    Expect(scene.phase==(uint8_t)FLOPPY144_GREY_DIALOGUE &&
        Floppy144GreyEncounterDisplayPercent(&scene)==27U,
        "BF35 dialogue starts with original true percentage");
    for(k=0U;k<31U;++k) {
        (void)Floppy144GreyEncounterAdvance(&scene,100U);
        Expect(Floppy144GreyEncounterDisplayPercent(&scene)==27U,
            "BF35 no recovery increase while developer speaks");
    }
    (void)Floppy144GreyEncounterAdvance(&scene,100U);
    Expect(scene.phase==(uint8_t)FLOPPY144_GREY_CAPACITY &&
        Floppy144GreyEncounterDisplayPercent(&scene)==27U,
        "BF35 recovery starts strictly after full dialogue");
    for(k=0U;k<10U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    early=Floppy144GreyEncounterDisplayPercent(&scene);
    for(k=0U;k<10U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    middle=Floppy144GreyEncounterDisplayPercent(&scene);
    for(k=0U;k<10U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    late=Floppy144GreyEncounterDisplayPercent(&scene);
    Expect(scene.phase==(uint8_t)FLOPPY144_GREY_GLITCH &&
        early>=27U && middle>early && late>middle &&
        (late-middle)>(middle-early),
        "BF35 recovery increment accelerates instead of moving linearly");
    for(k=0U;k<3U;++k) (void)Floppy144GreyEncounterAdvance(&scene,100U);
    Expect(Floppy144GreyEncounterDisplayPercent(&scene)==144U &&
        scene.phase==(uint8_t)FLOPPY144_GREY_GLITCH,
        "BF35 exactly 144 percent is visible before blackout");
    /* The original I/Inspect route remains separate from the idle timer. */
    manual.phase=(uint8_t)FLOPPY144_GREY_EXPLORE;
    manual.local_x=431; manual.local_y=264;
    Expect(Floppy144GreyEncounterInspect(&manual) &&
        manual.developer_inspected &&
        manual.phase==(uint8_t)FLOPPY144_GREY_IDENTIFY &&
        manual.elapsed_ms==0U,
        "BF35 approaching and inspecting first keeps identify phase");
    puts("BF35 Developer reaction and accelerating recovery timing PASS");
}

/* S4G-03: exhaustively enter from all candidate orientations, inspect the
   Developer and run through the full temporary scene without touching any
   permanent progress field. */
static void TestGreyEncounter(const char *root)
{
    static uint32_t pixels[640U*360U];
    static uint32_t bf35_snapshot[640U*360U];
    Floppy144Surface surface;
    Floppy144DiscoveryProfile profile, profile_before;
    Floppy144RunState run, before, restored;
    Floppy144GreyEncounter scene;
    Floppy144GreyDoorCandidate candidate;
    uint32_t i, k, phase_visited, baseline_checksum, render_checksum;
    uint32_t last_percent=0U, max_percent=0U;
    bool red_on=false,red_off=false;
    uint32_t count=Floppy144GreyDoorCandidateCount();
    char path[F144_PLATFORM_PATH_CAPACITY];

    TestBF35EncounterTiming();
    memset(&surface,0,sizeof(surface));
    surface.width=640U;
    surface.height=360U;
    surface.pixels=pixels;
    Floppy144DiscoveryProfileReset(&profile);
    profile_before=profile;
    Expect(TestJoinPath(root,"grey-encounter.sav",path,(uint32_t)sizeof(path)),
        "S4G-03 isolated test save path");

    for(i=0U;i<count;++i)
    {
        Floppy144RunStateBegin(&run,144U+i);
        (void)Floppy144RunStateBitSet(
            run.rooms,(uint32_t)FLOPPY144_ROOM_CORRIDOR);
        run.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE;
        Expect(Floppy144GreyDoorCandidateAt(i,&candidate),
            "S4G-03 every vetted wall candidate resolves");
        Floppy144RunStateSetPlayerSitePosition(
            &run,candidate.stand_x16,candidate.stand_y16);
        /*
         * Force each candidate through the same public seed selector without
         * adding test-only co-ordinate entry paths to the shipping game.
         */
        {
            uint32_t seed=0U,tries=0U;
            while(Floppy144RunStateGreyDoorPlacementSlot(&run,count)!=i &&
                  tries<20000U)
            {
                ++seed;
                ++tries;
                run.recovery_seed=seed;
            }
            Expect(tries<20000U,
                "S4G-03 each candidate is addressable by an ordinary run seed");
        }
        Expect(Floppy144GreyDoorNearby(&run),
            "S4G-03 physical A/Access range covers the selected wall stance");
        before=run;
        Expect(Floppy144GreyEncounterBegin(&scene,&run) &&
            !Floppy144GreyEncounterSaveAllowed(&scene) &&
            scene.return_x16==before.player_site_x &&
            scene.return_y16==before.player_site_y &&
            scene.restoration_start==Floppy144RunStateRecoveredPercent(&run) &&
            Floppy144GreyEncounterDisplayPercent(&scene)==
                Floppy144RunStateRecoveredPercent(&run),
            "BUG13 real run restoration and exact world return are snapshotted");
        Expect(!Floppy144GreyEncounterInspect(&scene) &&
            !Floppy144GreyEncounterMove(&scene,12,0),
            "S4G-03 entry transition ignores movement and premature Inspect");
        for(k=0;k<12U && scene.phase!=(uint8_t)FLOPPY144_GREY_EXPLORE;++k)
            (void)Floppy144GreyEncounterAdvance(&scene,80U);
        Expect(scene.phase==(uint8_t)FLOPPY144_GREY_EXPLORE,
            "S4G-03 transition reaches clean modern office");
        Floppy144GreyEncounterDraw(&surface,&scene);
        /* BF42: samples from the four projected rear-wall front faces,
           rendered in Explore before the later Developer speech overlay. */
        Expect(pixels[115U*640U+236U]==FLOPPY144_RGB(56,97,134),
            "BF42 west poster one uses projected face and original palette");
        Expect(pixels[153U*640U+96U]==FLOPPY144_RGB(110,64,105),
            "BF42 west poster two uses projected wall slope");
        Expect(pixels[163U*640U+579U]==FLOPPY144_RGB(50,111,92),
            "BF42 north poster is mapped to wall-mounted artwork");
        Expect(pixels[145U*640U+488U]==FLOPPY144_RGB(12,23,34),
            "BF42 projected source screen retains interior colours");
        /* BF35: passive reaction occurs at exactly 5000 ms. */
        {
            Floppy144GreyEncounter idle=scene;
            for(k=0U;k<49U;++k)
                (void)Floppy144GreyEncounterAdvance(&idle,100U);
            Expect(idle.phase==(uint8_t)FLOPPY144_GREY_EXPLORE &&
                idle.elapsed_ms==4900U &&
                Floppy144GreyEncounterDisplayPercent(&idle)==idle.restoration_start,
                "BF35 Developer remains occupied for the first 4900 ms");
            (void)Floppy144GreyEncounterAdvance(&idle,100U);
            Expect(idle.phase==(uint8_t)FLOPPY144_GREY_TURN &&
                idle.elapsed_ms==0U && idle.developer_inspected==0U,
                "BF35 automatic turn begins at 5000 ms without Inspect");
            for(k=0U;k<19U;++k)
                (void)Floppy144GreyEncounterAdvance(&idle,100U);
            Expect(idle.phase==(uint8_t)FLOPPY144_GREY_DIALOGUE &&
                Floppy144GreyEncounterDisplayPercent(&idle)==
                    idle.restoration_start,
                "BF35 idle Developer turn preserves original dialogue");
        }
        render_checksum=0U;
        for(k=0U;k<640U*360U;k+=113U)
            render_checksum=(render_checksum*33U)^pixels[k];
        Expect(render_checksum!=0U,
            "S4G-03 modern office procedurally fills framebuffer");

        Expect(!Floppy144GreyEncounterInspect(&scene) &&
            !Floppy144GreyEncounterInspectNearby(&scene),
            "BUG13 distant Developer has neither prompt nor interaction");
        /* Exactly 24 scene pixels is close-range; 25 is not. */
        scene.local_x=430;
        scene.local_y=264;
        Expect(!Floppy144GreyEncounterInspectNearby(&scene) &&
            !Floppy144GreyEncounterInspect(&scene),
            "BUG13 25-pixel distance cannot inspect Developer");
        scene.local_x=431;
        Expect(Floppy144GreyEncounterInspectNearby(&scene),
            "BUG13 24-pixel distance displays Inspect prompt");
        scene.local_x=115;
        scene.local_y=264;
        for(k=0U;k<29U;++k)
            (void)Floppy144GreyEncounterMove(&scene,12,0);
        Expect(Floppy144GreyEncounterInspect(&scene) &&
            scene.developer_inspected==1U &&
            scene.phase==(uint8_t)FLOPPY144_GREY_IDENTIFY,
            "S4G-03 approach then I/Inspect identifies DEVELOPER");
        phase_visited=0U;
        baseline_checksum=render_checksum;
        last_percent=scene.restoration_start;
        max_percent=last_percent;
        red_on=false;
        red_off=false;
        for(k=0U;k<200U&&!Floppy144GreyEncounterFinished(&scene);++k)
        {
            uint32_t percent=Floppy144GreyEncounterDisplayPercent(&scene);
            if(scene.phase<=(uint8_t)FLOPPY144_GREY_DIALOGUE)
                Expect(percent==scene.restoration_start,
                    "BF35 recovery percentage is frozen until dialogue ends");
            phase_visited|=1U<<scene.phase;
            Expect(percent>=scene.restoration_start && percent<=144U &&
                percent>=last_percent,
                "BUG13 restoration visual value monotonically accelerates 0-144");
            last_percent=percent;
            if(percent>max_percent) max_percent=percent;
            if(percent>100U) {
                if(Floppy144GreyEncounterPercentFlash(&scene))
                    red_on=true;
                else
                    red_off=true;
            } else {
                Expect(!Floppy144GreyEncounterPercentFlash(&scene),
                    "BUG13 only >100 percent flashes red");
            }
            Expect(!Floppy144GreyEncounterSaveAllowed(&scene),
                "S4G-03 no temporary scene phase permits save/autosave");
            Floppy144GreyEncounterDraw(&surface,&scene);
            if(scene.phase==(uint8_t)FLOPPY144_GREY_CAPACITY &&
               scene.elapsed_ms==1000U)
                memcpy(bf35_snapshot,pixels,sizeof(bf35_snapshot));
            Floppy144GreyEncounterDrawInterference(&surface,&scene);
            if(scene.phase==(uint8_t)FLOPPY144_GREY_CAPACITY &&
               scene.elapsed_ms==1000U) {
                Expect(memcmp(bf35_snapshot,pixels,sizeof(bf35_snapshot))!=0,
                    "BF35 interference begins while recovery is rising");
                memcpy(bf35_snapshot,pixels,sizeof(bf35_snapshot));
                Floppy144GreyEncounterDraw(&surface,&scene);
                Floppy144GreyEncounterDrawInterference(&surface,&scene);
                Expect(memcmp(bf35_snapshot,pixels,sizeof(bf35_snapshot))==0,
                    "BF35 corruption is deterministic and frame-local");
            }
            if(scene.phase==(uint8_t)FLOPPY144_GREY_BLACKOUT)
            {
                uint32_t pixel;
                for(pixel=0U;pixel<640U*360U;++pixel)
                    Expect(pixels[pixel]==FLOPPY144_RGB(0,0,0),
                        "BUG13 blackout covers every framebuffer pixel");
                Expect(scene.elapsed_ms<2000U,
                    "BUG13 blackout remains active for its full two seconds");
            }
            (void)Floppy144GreyEncounterAdvance(&scene,100U);
        }
        Expect(max_percent==144U && red_on && red_off,
            "BUG13 reaches 144 from actual progress and >100 flashes readably");
        Expect(Floppy144GreyEncounterFinished(&scene) &&
            (phase_visited&(1U<<FLOPPY144_GREY_IDENTIFY))!=0U &&
            (phase_visited&(1U<<FLOPPY144_GREY_TURN))!=0U &&
            (phase_visited&(1U<<FLOPPY144_GREY_DIALOGUE))!=0U &&
            (phase_visited&(1U<<FLOPPY144_GREY_CAPACITY))!=0U &&
            (phase_visited&(1U<<FLOPPY144_GREY_GLITCH))!=0U &&
            (phase_visited&(1U<<FLOPPY144_GREY_BLACKOUT))!=0U,
            "S4G-03 every authored encounter stage occurs in order");
        Expect(Floppy144GreyEncounterSaveAllowed(&scene),
            "S4G-03 temporary save guard releases only after vignette finishes");
        Expect(memcmp(&run,&before,sizeof(run))==0 &&
            memcmp(&profile,&profile_before,sizeof(profile))==0,
            "S4G-03 full scene does not modify run, capacity, evidence, completion or Profile");
        Expect(Floppy144RunStateGreyDoorComplete(&run) &&
            run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
            !Floppy144GreyDoorNearby(&run) &&
            !Floppy144GreyDoorForRun(&run,&candidate) &&
            run.player_site_x==scene.return_x16 &&
            run.player_site_y==scene.return_y16,
            "S4G-03 only final completion bit changes and original wall/position return");
        Expect(Floppy144PersistenceSaveRunState(path,&run) &&
            Floppy144PersistenceLoadRunState(path,&restored) &&
            restored.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
            !Floppy144GreyDoorForRun(&restored,&candidate) &&
            !Floppy144GreyEncounterBegin(&scene,&restored),
            "S4G-03 saved completed event cannot ever re-enter");
        (void)baseline_checksum; /* render checksum is scene-only. */
    }
    printf("S4G-03 encounter audit: %u candidate entrances, "
        "developer, 144%% gag, glitch, save suppression, permanent return\n",
        (unsigned)count);
}


/* S4G-04: entire run-level state machine, permanent save state, unrelated
   recovery independence, and deterministic location through room changes. */
static void TestGreyDoorOneShotLifecycle(const char *root)
{
    Floppy144RunState run={0},fresh={0},manual={0},autosave={0},
        completed={0},reloaded={0};
    Floppy144DiscoveryProfile profile,profile_before;
    Floppy144GreyDoorCandidate door={0};
    uint32_t i,seed=9441U,count=Floppy144GreyDoorCandidateCount();
    uint32_t initial_rect_count=Floppy144SiteRectCount();
    uint32_t initial_collections;
    uint32_t selected_index;
    int32_t px,py;
    char path[F144_PLATFORM_PATH_CAPACITY];

    Floppy144DiscoveryProfileReset(&profile);
    profile_before=profile;
    Floppy144RunStateBegin(&run,seed);
    initial_collections=Floppy144RunStateRecoveredKb(&run);
    Expect(run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
        !Floppy144GreyDoorForRun(&run,&door) &&
        !Floppy144GreyDoorNearby(&run) &&
        Floppy144SiteRectCount()==initial_rect_count,
        "S4G-04 pristine run has zero Door rendering/hook/collision geometry");

    Expect(TestJoinPath(root,"grey-one-shot.sav",path,
        (uint32_t)sizeof(path)),"S4G-04 save path");
    Expect(Floppy144PersistenceSaveRunState(path,&run) &&
        Floppy144PersistenceLoadRunState(path,&reloaded) &&
        reloaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
        !Floppy144GreyDoorForRun(&reloaded,&door),
        "S4G-04 UNSEEN save/reload is invisible");

    (void)Floppy144RunStateReconstructRoom(
        &run,FLOPPY144_ROOM_CORRIDOR);
    Expect(!Floppy144GreyDoorForRun(&run,&door),
        "S4G-04 corridor reconstructed BEFORE discovery still has no door");
    selected_index=Floppy144RunStateGreyDoorPlacementSlot(&run,count);
    Expect(Floppy144GreyDoorCandidateAt(selected_index,&door),
        "S4G-04 deterministic candidate resolves");
    Floppy144RunStateSetPlayerSitePosition(
        &run,door.stand_x16,door.stand_y16);
    px=run.player_site_x;py=run.player_site_y;
    Expect(!Floppy144GreyDoorNearby(&run),
        "S4G-04 before record, even standing at chosen wall cannot Inspect");
    Expect(Floppy144RunStateGreyDoorDiscover(&run) &&
        !Floppy144RunStateGreyDoorDiscover(&run) &&
        Floppy144GreyDoorNearby(&run) &&
        Floppy144GreyDoorForRun(&run,&door),
        "S4G-04 first record opens one persistent logical Grey Door hook");

    /* The orphan document is legitimately accessible only after DR-01;
       recording this state also tests terminal re-entry after completion. */
    (void)Floppy144RunStateBitSet(
        run.collections,(uint32_t)FLOPPY144_COLLECTION_DR01);
    manual=run;
    run.dirty=0U;
    (void)Floppy144RunStateReconstructRoom(&run,FLOPPY144_ROOM_SECURITY);
    Expect(Floppy144GreyDoorForRun(&run,&door) &&
        Floppy144RunStateGreyDoorPlacementSlot(&run,count)==selected_index,
        "S4G-04 reconstructing other rooms never moves or deletes Door");
    Expect(Floppy144RunStateReconstructRoom(
        &run,FLOPPY144_ROOM_RECEPTION),
        "S4G-04 unrelated reception room reconstructs normally");
    {
        int32_t rx,ry;
        Floppy144SiteSpawnPosition(&rx,&ry);
        Floppy144RunStateSetPlayerSitePosition(&run,rx,ry);
        Expect(!Floppy144GreyDoorNearby(&run) &&
            Floppy144GreyDoorForRun(&run,&door),
            "S4G-04 remote room has no interaction, Door stays in corridor");
    }
    Floppy144RunStateSetPlayerSitePosition(&run,px,py);
    Expect(Floppy144GreyDoorNearby(&run) &&
        Floppy144RunStateGreyDoorPlacementSlot(&run,count)==selected_index,
        "S4G-04 corridor re-entry preserves same unique seeded location");

    /*
     * The canonical Site collision system includes floor-footprint and
     * room-topology checks, rather than treating every drawn wall cell as a
     * low-level blocking rectangle. Compare actual movement from the same
     * reachable Corridor stance across all three states. The anomaly must
     * neither create a passage nor obstruct one that was already valid.
     */
    {
        Floppy144RunState unseen=run,available=run,done=run;
        int32_t dx=door.rect.width>door.rect.height
            ? 0 : FLOPPY144_SITE_MOVE_STEP_X16;
        int32_t dy=door.rect.width>door.rect.height
            ? -FLOPPY144_SITE_MOVE_STEP_X16 : 0;
        unseen.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE;
        done.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED;
        for(i=0U;i<12U;++i)
        {
            bool old_move=Floppy144RunStateMovePlayerSite(&unseen,dx,dy);
            bool open_move=Floppy144RunStateMovePlayerSite(&available,dx,dy);
            bool gone_move=Floppy144RunStateMovePlayerSite(&done,dx,dy);
            Expect(old_move==open_move && old_move==gone_move &&
                unseen.player_site_x==available.player_site_x &&
                available.player_site_x==done.player_site_x &&
                unseen.player_site_y==available.player_site_y &&
                available.player_site_y==done.player_site_y,
                "S4G-04 wall movement/collision identical across all three states");
        }
    }
    Expect(Floppy144SiteRectCount()==initial_rect_count,
        "S4G-04 no Grey Door geometry is inserted into the Site");

    Expect(Floppy144PersistenceSaveRunState(path,&run) &&
        Floppy144PersistenceLoadRunState(path,&reloaded) &&
        Floppy144GreyDoorForRun(&reloaded,&door) &&
        Floppy144RunStateGreyDoorPlacementSlot(&reloaded,count)==selected_index,
        "S4G-04 AVAILABLE save/reload preserves exact deterministic position");

    Expect(Floppy144RunStateGreyDoorComplete(&run) &&
        run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
        !Floppy144GreyDoorForRun(&run,&door) &&
        !Floppy144GreyDoorNearby(&run) &&
        Floppy144SiteRectCount()==initial_rect_count,
        "S4G-04 COMPLETED is a silent, geometry-free permanent state");
    run.dirty=0U;
    completed=run;
    for(i=0U;i<3U;++i)
    {
        Expect(Floppy144DocumentApplyEffects(NULL,&run,
            FLOPPY144_GREY_DOOR_RECORD_COLLECTION,
            FLOPPY144_GREY_DOOR_RECORD_INDEX) &&
            memcmp(&run,&completed,sizeof(run))==0,
            "S4G-04 repeated terminal/document views NEVER resurrect completed Door");
    }
    Expect(!Floppy144RunStateGreyDoorDiscover(&run) &&
        !Floppy144RunStateGreyDoorComplete(&run) &&
        run.dirty==0U,
        "S4G-04 completed->available and completed->completed are forbidden");
    Expect(Floppy144PersistenceSaveRunState(path,&run) &&
        Floppy144PersistenceLoadRunState(path,&reloaded) &&
        reloaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
        !Floppy144GreyDoorForRun(&reloaded,&door) &&
        !Floppy144GreyDoorNearby(&reloaded),
        "S4G-04 completed restart/save does not recreate Door");
    (void)Floppy144RunStateReconstructRoom(
        &reloaded,FLOPPY144_ROOM_MAIN_OFFICE);
    Expect(!Floppy144GreyDoorForRun(&reloaded,&door),
        "S4G-04 later room restoration cannot resurrect completed Door");

    /* Narrow priority exception: matching run, completed autosave wins.
       Other runs, even if they carry an available Door, remain independent. */
    autosave=run;
    Expect(Floppy144RunStateGreyDoorCompletedAutosavePreferred(
        &manual,&autosave),
        "S4G-04 completed autosave takes precedence over pre-event manual");
    Expect(!Floppy144RunStateGreyDoorCompletedAutosavePreferred(
        &autosave,&manual) &&
        !Floppy144RunStateGreyDoorCompletedAutosavePreferred(
            &autosave,&autosave),
        "S4G-04 completed manual cannot be demoted");
    autosave.recovery_seed++;
    Expect(!Floppy144RunStateGreyDoorCompletedAutosavePreferred(
        &manual,&autosave),
        "S4G-04 unrelated run seed cannot hijack a recorded session");
    autosave=manual;
    Expect(!Floppy144RunStateGreyDoorCompletedAutosavePreferred(
        &manual,&autosave),
        "S4G-04 ordinary manual checkpoint keeps precedence");

    Floppy144RunStateBegin(&fresh,seed+1U);
    Expect(fresh.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
        !Floppy144GreyDoorForRun(&fresh,&door) &&
        Floppy144RunStateRecoveredKb(&fresh)==initial_collections &&
        !Floppy144RunStateCollectionRestored(&fresh,FLOPPY144_COLLECTION_DR01) &&
        memcmp(&profile,&profile_before,sizeof(profile))==0,
        "S4G-04 fresh recovery restores opportunity without altering Profile");
    Floppy144RunStateBegin(&fresh,seed);
    Expect(fresh.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
        !Floppy144GreyDoorForRun(&fresh,&door),
        "S4G-04 explicit new run resets one-shot even with deterministic seed override");
    (void)Floppy144RunStateReconstructRoom(&fresh,FLOPPY144_ROOM_CORRIDOR);
    Expect(Floppy144RunStateGreyDoorDiscover(&fresh) &&
        Floppy144GreyDoorForRun(&fresh,&door),
        "S4G-04 independent new run can discover Grey Door afresh");
    printf("S4G-04 lifecycle hardening: UNSEEN/AVAILABLE/COMPLETED, "
        "manual/autosave priority, wall/no-scar, new-run reset\n");
}


/*
 * S4G-05: the full gate, with ACTUAL disk round-trips at the three persistent
 * boundaries. This exercises all viable secret-stage entry points with seeded
 * early, middle, late and low-headroom run snapshots. The scene never has a
 * writable path back into normal state.
 *
 * The later contexts are deliberately synthetic snapshots, not a replacement
 * for the Stage 3 full gameplay traversal regressions.
 */
static void TestStage4GCompleteJourney(const char *root)
{
    static const uint32_t seeds[]={
        0U,1U,42U,144U,146U,2026U,65535U,
        0x12345678U,0xffffffffU,9441U,271828U,314159U
    };
    static uint32_t pixels[640U*360U];
    Floppy144Surface surface;
    Floppy144RunState run={0},original={0},restored={0},
        expected={0},manual={0},completed={0};
    Floppy144GreyDoorCandidate candidate={0},chosen={0};
    Floppy144GreyEncounter scene;
    Floppy144DiscoveryProfile profile,profile_before;
    char save_path[F144_PLATFORM_PATH_CAPACITY];
    char auto_path[F144_PLATFORM_PATH_CAPACITY];
    uint32_t candidate_count=Floppy144GreyDoorCandidateCount();
    uint32_t seed_i,variant,first_slot=0U,different=0U,total=0U;
    uint32_t rect_count=Floppy144SiteRectCount();
    uint32_t initial_bytes=0U,baseline_percent=0U;
    bool baseline_evidence=false,baseline_exhaustion=false;

    memset(&surface,0,sizeof(surface));
    surface.width=640U;
    surface.height=360U;
    surface.pixels=pixels;
    Expect(TestJoinPath(root,"stage4g-full-manual.sav",save_path,
        (uint32_t)sizeof(save_path)) &&
        TestJoinPath(root,"stage4g-full-autosave.sav",auto_path,
        (uint32_t)sizeof(auto_path)),
        "S4G-05 full journey has isolated physical save paths");
    Expect(candidate_count>=2U,"S4G-05 has multiple safe derived wall slots");

    for(seed_i=0U;seed_i<(uint32_t)(sizeof(seeds)/sizeof(seeds[0]));++seed_i)
    for(variant=0U;variant<4U;++variant)
    {
        uint32_t slot,k;
        uint32_t entered_phase=0U;
        Floppy144CollectionId collection=FLOPPY144_COLLECTION_DR01;
        uint32_t index=0U;
        Floppy144RunStateBegin(&run,seeds[seed_i]);
        Floppy144DiscoveryProfileReset(&profile);
        (void)Floppy144DiscoveryProfileSetBodyStyle(
            &profile,FLOPPY144_OPERATOR_BODY_STYLE_B);
        profile.recovery_sessions_begun=3U;
        profile.completed_recoveries=1U;
        profile_before=profile;

        /* DR-01 is restored in every legitimate route to the hidden record. */
        (void)Floppy144RunStateBitSet(
            run.collections,(uint32_t)FLOPPY144_COLLECTION_DR01);
        (void)Floppy144RunStateReconstructRoom(
            &run,FLOPPY144_ROOM_CORRIDOR);
        if(variant>0U)
        {
            /* Intermediate reconstructed rooms and distinct ending routes. */
            (void)Floppy144RunStateReconstructRoom(
                &run,FLOPPY144_ROOM_MAIN_OFFICE);
            (void)Floppy144RunStateBitSet(run.collections,
                (uint32_t)FLOPPY144_COLLECTION_DR02);
            (void)Floppy144RunStateBitSet(run.evidence,0U);
            (void)Floppy144RunStateBitSet(run.notebook,0U);
            run.notebook_order_count=1U;
            run.notebook_order[0]=0U;
            run.act=(uint8_t)(variant==1U?FLOPPY144_RUN_ACT_I:
                variant==2U?FLOPPY144_RUN_ACT_II:FLOPPY144_RUN_ACT_III);
            run.branch=(uint8_t)(variant==2U?
                FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST:
                FLOPPY144_RUN_BRANCH_RECORDS_FIRST);
        }
        if(variant==3U)
        {
            /* Exercise the transition very near the real 1,440 KB ceiling.
               Use source collection weights, never fake the capacity. */
            for(k=0U;k<(uint32_t)FLOPPY144_COLLECTION_COUNT;++k)
            {
                const Floppy144CollectionDefinition *def;
                uint32_t before_used;
                if(k==(uint32_t)FLOPPY144_COLLECTION_DR01 ||
                   k==(uint32_t)FLOPPY144_COLLECTION_DR02) continue;
                def=Floppy144CollectionGet((Floppy144CollectionId)k);
                if(def==NULL) continue;
                before_used=Floppy144RunStateRecoveredKb(&run);
                if(before_used+def->size_kb<=FLOPPY144_RECOVERY_CAPACITY_KB)
                    (void)Floppy144RunStateBitSet(run.collections,k);
            }
            Expect(Floppy144RunStateFreeKb(&run)<250U,
                "S4G-05 synthetic late route is close to capacity exhaustion");
        }

        /* Save before trigger: there is no display/hook and restoring does
           not independently discover the door. */
        Expect(run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
            !Floppy144GreyDoorForRun(&run,&candidate) &&
            Floppy144PersistenceSaveRunState(save_path,&run) &&
            Floppy144PersistenceLoadRunState(save_path,&restored) &&
            restored.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
            !Floppy144GreyDoorForRun(&restored,&candidate),
            "S4G-05 UNAVAILABLE manual checkpoint and app restart remain hidden");
        run=restored;

        Expect(Floppy144DocumentFindRecordId(
            FLOPPY144_GREY_DOOR_RECORD_ID,&collection,&index) &&
            collection==FLOPPY144_GREY_DOOR_RECORD_COLLECTION &&
            index==FLOPPY144_GREY_DOOR_RECORD_INDEX &&
            Floppy144DocumentAccessible(&run,collection,index) &&
            Floppy144DocumentApplyEffects(NULL,&run,collection,index) &&
            run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE,
            "S4G-05 reachable authored document alone activates one Door");
        slot=Floppy144RunStateGreyDoorPlacementSlot(&run,candidate_count);
        if(seed_i==0U) first_slot=slot;
        if(slot!=first_slot) different=1U;
        Expect(Floppy144GreyDoorForRun(&run,&candidate) &&
            Floppy144GreyDoorCandidateSafe(&candidate) &&
            Floppy144GreyDoorCandidateAt(slot,&chosen) &&
            memcmp(&candidate,&chosen,sizeof(candidate))==0,
            "S4G-05 selected placement is stable and independently safe");
        (void)Floppy144VariationValue(seeds[seed_i],
            "staff.noticeboard.annotation.v1","AMB-NB-07");
        Expect(slot==Floppy144RunStateGreyDoorPlacementSlot(
                &run,candidate_count),
            "S4G-05 other seeded features cannot reroll Grey Door");

        /* Save AFTER trigger and restore the actual V3 file. */
        Expect(Floppy144PersistenceSaveRunState(auto_path,&run) &&
            Floppy144PersistenceLoadRunState(auto_path,&restored) &&
            restored.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE &&
            Floppy144RunStateGreyDoorPlacementSlot(
                &restored,candidate_count)==slot,
            "S4G-05 AVAILABLE autosave and process restart retain identical door");
        run=restored;
        Floppy144RunStateSetPlayerSitePosition(
            &run,candidate.stand_x16,candidate.stand_y16);
        Expect(Floppy144GreyDoorNearby(&run),
            "S4G-05 selected Door can physically be accessed");
        original=run;
        initial_bytes=Floppy144RunStateRecoveredKb(&run);
        baseline_percent=Floppy144RunStateRecoveredPercent(&run);
        baseline_evidence=Floppy144GameDataEvidenceResolved(&run);
        baseline_exhaustion=
            Floppy144RunStateAvailableRecoveryCapacityExhausted(&run);
        Expect(baseline_percent<=100U,
            "S4G-05 real restored percentage never reaches visual-only 144%%");

        Expect(Floppy144GreyEncounterBegin(&scene,&run) &&
            !Floppy144GreyEncounterSaveAllowed(&scene) &&
            scene.restoration_start==baseline_percent &&
            Floppy144GreyEncounterDisplayPercent(&scene)==baseline_percent,
            "BUG13 all seeded routes start HUD from the genuine restoration");
        for(k=0U;k<12U && scene.phase!=(uint8_t)FLOPPY144_GREY_EXPLORE;++k)
            (void)Floppy144GreyEncounterAdvance(&scene,100U);
        Expect(scene.phase==(uint8_t)FLOPPY144_GREY_EXPLORE,
            "S4G-05 transition reaches impossible office");
        while(scene.local_x<440)
            (void)Floppy144GreyEncounterMove(&scene,12,0);
        Expect(Floppy144GreyEncounterInspect(&scene) &&
            scene.phase==(uint8_t)FLOPPY144_GREY_IDENTIFY,
            "S4G-05 I/Inspect begins Developer sequence");

        for(k=0U;k<200U && !Floppy144GreyEncounterFinished(&scene);++k)
        {
            entered_phase |= 1U<<scene.phase;
            Expect(!Floppy144GreyEncounterSaveAllowed(&scene),
                "S4G-05 autosave/manual save prohibited inside all temporary phases");
            Floppy144GreyEncounterDraw(&surface,&scene);
            (void)Floppy144GreyEncounterAdvance(&scene,100U);
        }
        Expect(Floppy144GreyEncounterFinished(&scene) &&
            (entered_phase&(1U<<FLOPPY144_GREY_IDENTIFY))!=0U &&
            (entered_phase&(1U<<FLOPPY144_GREY_TURN))!=0U &&
            (entered_phase&(1U<<FLOPPY144_GREY_DIALOGUE))!=0U &&
            (entered_phase&(1U<<FLOPPY144_GREY_CAPACITY))!=0U &&
            (entered_phase&(1U<<FLOPPY144_GREY_GLITCH))!=0U &&
            Floppy144GreyEncounterSaveAllowed(&scene),
            "S4G-05 office, Developer, exact dialogue, 144%% and CRT stages finish");

        /* Absolutely no real state mutated during rendering or animation.
           Completion may change only the hidden flag and dirty bit. */
        Expect(memcmp(&run,&original,sizeof(run))==0 &&
            memcmp(&profile,&profile_before,sizeof(profile))==0 &&
            Floppy144RunStateRecoveredKb(&run)==initial_bytes &&
            Floppy144RunStateRecoveredPercent(&run)==baseline_percent &&
            Floppy144GameDataEvidenceResolved(&run)==baseline_evidence &&
            Floppy144RunStateAvailableRecoveryCapacityExhausted(&run)==
                baseline_exhaustion &&
            Floppy144SiteRectCount()==rect_count,
            "S4G-05 game progress, final routes, Profile and geometry untouched");

        expected=original;
        expected.grey_door_state=(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED;
        expected.dirty=1U;
        Expect(Floppy144RunStateGreyDoorComplete(&run) &&
            memcmp(&run,&expected,sizeof(run))==0 &&
            scene.return_x16==run.player_site_x &&
            scene.return_y16==run.player_site_y &&
            !Floppy144GreyDoorForRun(&run,&candidate) &&
            !Floppy144GreyDoorNearby(&run),
            "S4G-05 only hidden lifecycle flag and dirty change on corridor return");

        /* Autosave resumes IMMEDIATELY after return, and old same-seed
           pre-trigger manual saves cannot reactivate a completed run. */
        Expect(Floppy144GreyEncounterSaveAllowed(&scene) &&
            Floppy144PersistenceSaveRunState(auto_path,&run) &&
            Floppy144PersistenceLoadRunState(auto_path,&completed) &&
            completed.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_COMPLETED &&
            Floppy144PersistenceLoadRunState(save_path,&manual) &&
            Floppy144RunStateGreyDoorCompletedAutosavePreferred(
                &manual,&completed),
            "S4G-05 completed autosave beats earlier same-seed manual after restart");
        run=completed;
        run.dirty=0U;
        expected=run;
        Expect(Floppy144DocumentApplyEffects(NULL,&run,collection,index) &&
            memcmp(&run,&expected,sizeof(run))==0 &&
            !Floppy144GreyDoorForRun(&run,&candidate) &&
            !Floppy144GreyEncounterBegin(&scene,&run),
            "S4G-05 reopened terminal document cannot replay a completed encounter");
        Expect(memcmp(&profile,&profile_before,sizeof(profile))==0 &&
            Floppy144RunStateRecoveredKb(&run)==initial_bytes &&
            Floppy144RunStateRecoveredPercent(&run)==baseline_percent &&
            Floppy144GameDataEvidenceResolved(&run)==baseline_evidence &&
            Floppy144RunStateAvailableRecoveryCapacityExhausted(&run)==
                baseline_exhaustion,
            "S4G-05 no changes to Profile, ending flags or final percentages");
        ++total;
        if(variant==0U)
            printf("S4G-05 seed=%u slot=%u wall=(%u,%u %ux%u) free=%u KB\n",
                (unsigned)seeds[seed_i],(unsigned)slot,
                (unsigned)chosen.rect.x,(unsigned)chosen.rect.y,
                (unsigned)chosen.rect.width,(unsigned)chosen.rect.height,
                (unsigned)Floppy144RunStateFreeKb(&run));
    }
    Expect(different!=0U,
        "S4G-05 two or more fixed seeds produce different wall locations");
    printf("S4G-05 full journey: %u seeded route scenarios, %u valid candidates, "
        "all three physical save boundaries, one-shot and neutral endings PASS\n",
        (unsigned)total,(unsigned)candidate_count);
}


/* S4H: Hathaway uses an isolated, RAM-only all-revealed snapshot. The
   directory-adjacent Door is a fixed presentation override and can be
   inspected repeatedly without consuming the saved Grey Door lifecycle. */
static void TestStage4HInspection(const char *root)
{
    Floppy144RunState run={0},normal={0},reloaded={0};
    Floppy144GreyDoorCandidate doorway={0},seeded={0};
    Floppy144GreyEncounter scene;
    uint32_t i,visited=0U;
    char path[F144_PLATFORM_PATH_CAPACITY];
    Floppy144RunStateBegin(&run,144U);
    normal=run;
    Expect(Floppy144GreyDoorHathawayCandidate(&doorway) &&
        doorway.rect.x==29U && doorway.rect.y==56U &&
        doorway.rect.width==4U && doorway.rect.height==1U &&
        !Floppy144SitePositionBlocked(doorway.stand_x16,doorway.stand_y16),
        "S4H fixed visual Door is beside actual Site Directory and reachable");
    Expect(!Floppy144GreyDoorForRun(&run,&seeded) &&
        run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE,
        "S4H baseline Door remains undiscovered");
    Floppy144RunStateEnableHathawayInspection(&run);
    Expect(run.hathaway_inspection==1U &&
        run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE,
        "S4H synthetic inspection flag cannot discover a normal Door");
    for(i=0U;i<(uint32_t)FLOPPY144_COLLECTION_COUNT;++i)
        Expect(Floppy144RunStateCollectionRestored(
            &run,(Floppy144CollectionId)i),
            "S4H all collections represented as restored");
    for(i=0U;i<(uint32_t)FLOPPY144_ROOM_COUNT;++i)
        Expect(Floppy144RunStateRoomReconstructed(
            &run,(Floppy144RoomId)i),"S4H all rooms visible");
    for(i=0U;i<FLOPPY144_SECURE_CABINET_MAX;++i)
        Expect(Floppy144RunStateSecureCabinetUnlocked(&run,i),
            "S4H every secure cabinet is unlocked");
    {
        uint32_t record,found=0U,visible=0U;
        for(record=0U;record<Floppy144GameDataRecordCount();++record)
        {
            const Floppy144DataRecord *pi=
                Floppy144GameDataRecordAt(record);
            if(pi==NULL || pi->eKind!=FLOPPY144_DATA_PHYSICAL_ITEM)
                continue;
            ++found;
            if(Floppy144SitePhysicalItemVisible(&run,pi)) ++visible;
        }
        Expect(found>=500U && visible==found,
            "S4H every authored physical item is inspectable in restored world");
    }
    Expect(Floppy144GreyDoorForRun(&run,&seeded) &&
        memcmp(&seeded,&doorway,sizeof(doorway))==0,
        "S4H fixed Door overrides normal seeded selection");
    Floppy144RunStateSetPlayerSitePosition(
        &run,doorway.stand_x16,doorway.stand_y16);
    Expect(Floppy144GreyDoorNearby(&run),
        "S4H forced door offers ordinary proximity interaction");
    for(i=0U;i<2U;++i)
    {
        uint32_t frame=0U;
        Expect(Floppy144GreyEncounterBegin(&scene,&run),
            "S4H repeated scene entry works");
        while(scene.phase!=(uint8_t)FLOPPY144_GREY_EXPLORE &&
              frame++<50U)
            (void)Floppy144GreyEncounterAdvance(&scene,100U);
        while(scene.local_x<440)
            (void)Floppy144GreyEncounterMove(&scene,12,0);
        Expect(Floppy144GreyEncounterInspect(&scene),
            "S4H Developer inspection works repeatedly");
        for(frame=0U;frame<200U&&!Floppy144GreyEncounterFinished(&scene);++frame)
            (void)Floppy144GreyEncounterAdvance(&scene,100U);
        Expect(Floppy144GreyEncounterFinished(&scene) &&
            run.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE &&
            Floppy144GreyDoorForRun(&run,&seeded) &&
            Floppy144GreyDoorNearby(&run),
            "S4H scene returns to corridor and door remains indefinitely");
        ++visited;
    }
    Expect(visited==2U,"S4H forced Door was entered and exited twice");
    Expect(TestJoinPath(root,"hathaway-never-saved.sav",path,
        (uint32_t)sizeof(path)) &&
        Floppy144PersistenceSaveRunState(path,&normal) &&
        Floppy144PersistenceLoadRunState(path,&reloaded) &&
        reloaded.hathaway_inspection==0U &&
        reloaded.grey_door_state==(uint8_t)FLOPPY144_GREY_DOOR_UNAVAILABLE,
        "S4H ordinary persisted run never inherits inspection mode");
    /* Even a raw save codec cannot reconstruct the non-persistent flag.
       The coordinator separately prohibits actually writing it in Hathaway. */
    Expect(Floppy144RunStateRecoveredPercent(&normal)==0U &&
        !Floppy144GreyDoorForRun(&reloaded,&seeded),
        "S4H normal restored game remains a fresh recovery");
    puts("S4H: 35-collection/11-room inspector, all cabinets, fixed reusable Door, normal-save isolation PASS");
}


/* BF18: all actual document texts carry unique, first-open Notebook summaries. */
static void TestBF18DocumentNotebook(const char *root)
{
    Floppy144RunState run,loaded,legacy;
    char path[F144_PLATFORM_PATH_CAPACITY];
    uint8_t old_v4[FLOPPY144_SAVE_PAYLOAD_V4_SIZE];
    const Floppy144DataRecord *entries[192]={0};
    uint32_t found=0U,idx;
    uint32_t baseline;

    Floppy144RunStateBegin(&run,180U);
    baseline=Floppy144GameDataNotebookOrderedCount(&run);
    for(idx=0U;idx<Floppy144GameDataRecordCount();++idx)
    {
        const Floppy144DataRecord *n=Floppy144GameDataRecordAt(idx);
        if(n==NULL || n->eKind!=FLOPPY144_DATA_NOTEBOOK ||
           n->pszB==NULL || strcmp(n->pszB,"DOCUMENT")!=0)continue;
        if(found>=192U)
        {
            Expect(false,"BF18 document summaries fit fixed runtime capacity");
            break;
        }
        entries[found++]=n;
    }
    Expect(found==170U,"BF18 every one of 170 authored documents has a Notebook summary");
    Expect(run.document_note_count==0U &&
           Floppy144GameDataNotebookOrderedCount(&run)==baseline,
           "BF18 fresh run does not reveal unread authored summaries");
    for(idx=0U;idx<found;++idx)
    {
        const Floppy144DataRecord *n=entries[idx];
        Floppy144CollectionId collection;
        uint32_t slot,other;
        const Floppy144DataRecord *shown;
        uint32_t before;
        size_t id_length;
        Expect(n->pszId!=NULL && n->pszA!=NULL && n->pszC!=NULL,
               "BF18 authored summary metadata is complete");
        if(n->pszId==NULL||n->pszA==NULL)continue;
        id_length=strlen(n->pszId);
        Expect(n->n0==(int32_t)idx &&
               strncmp(n->pszA,n->pszId,id_length)==0 &&
               n->pszA[id_length]==':' && n->pszA[id_length+1]==' ' &&
               strlen(n->pszA)>id_length+35U,
               "BF18 Notebook sentence has exact canonical ID and substantive text");
        for(other=0U;other<idx;++other)
        {
            const Floppy144DataRecord *prior=entries[other];
            Expect(strcmp(n->pszId,prior->pszId)!=0 &&
                   strcmp(n->pszA+id_length+2U,
                      prior->pszA+strlen(prior->pszId)+2U)!=0,
                   "BF18 Notebook IDs and document prose remain unique");
        }
        Expect(Floppy144DocumentFindRecordId(n->pszId,&collection,&slot),
               "BF18 Notebook ID resolves to authored document registry");
        if(!Floppy144DocumentFindRecordId(n->pszId,&collection,&slot))
            continue;
        Expect(!Floppy144RunStateRecordOpened(&run,collection,slot),
               "BF18 unopened document has no canonical OPENED bit");
        before=Floppy144GameDataNotebookOrderedCount(&run);
        Expect(Floppy144GameDataRecordOpened(&run,collection,slot) &&
               Floppy144RunStateRecordOpened(&run,collection,slot),
               "BF18 first opening persists canonical record state");
        shown=Floppy144GameDataNotebookOrderedEntryAt(&run,before);
        Expect(shown!=NULL && shown->eKind==FLOPPY144_DATA_NOTEBOOK &&
               shown->pszB!=NULL && strcmp(shown->pszB,"DOCUMENT")==0,
               "BF18 first opening appends an authored document Notebook entry");
        Expect(!Floppy144GameDataRecordOpened(&run,collection,slot) &&
               Floppy144GameDataNotebookOrderedCount(&run)==before+1U,
               "BF18 re-opening never duplicates a Notebook entry");
    }
    Expect(run.document_note_count==170U &&
           Floppy144GameDataNotebookOrderedCount(&run)==baseline+170U,
           "BF18 all authored documents can be recorded beyond 128 old entries");
    Expect(!Floppy144RunStateRecordOpened(&run,FLOPPY144_COLLECTION_DR01,10U) &&
           Floppy144GameDataRecordOpened(&run,FLOPPY144_COLLECTION_DR01,10U) &&
           run.document_note_count==170U,
           "BF34 procedural records share OPENED bits without invented authored notes");

    Expect(TestJoinPath(root,"bf18-notebook.sav",path,
           (uint32_t)sizeof(path)) &&
           Floppy144PersistenceSaveRunState(path,&run) &&
           Floppy144PersistenceLoadRunState(path,&loaded),
           "BF18 save/load succeeds with complete document history");
    Expect(memcmp(run.records_opened,loaded.records_opened,
              sizeof(run.records_opened))==0 &&
           memcmp(run.document_note_order,loaded.document_note_order,
              sizeof(run.document_note_order))==0 &&
           run.document_note_count==loaded.document_note_count &&
           Floppy144GameDataNotebookOrderedCount(&loaded)==baseline+170U,
           "BF18 save/reload preserves viewed flags, ordering and Notebook contents");

    Expect(Floppy144PersistenceEncodeRunState(&run,old_v4,
           FLOPPY144_SAVE_PAYLOAD_V4_SIZE) &&
           Floppy144PersistenceDecodeRunState(&legacy,old_v4,
           FLOPPY144_SAVE_PAYLOAD_V4_SIZE) &&
           legacy.document_note_count==0U &&
           !Floppy144RunStateRecordOpened(&legacy,FLOPPY144_COLLECTION_DR01,10U) &&
           legacy.notebook_order_count==run.notebook_order_count,
           "BF18 V4 decoding preserves original Notebook while new status defaults unread");
    puts("BF18: 170 authored summaries, exact IDs, uniqueness, opened state, repeat view, V4/V5 persistence PASS");
}



/* BF29/BF31/BF32/BF34: exercise the same production record and size functions
 * that the document browser and document viewer share. */
static bool TestTitleUniqueWords(const char *title)
{
    const unsigned char *p=(const unsigned char *)title;
    while(*p)
    {
        const unsigned char *end,*q;
        while(*p && !isalnum(*p)) ++p;
        if(!*p) break;
        end=p; while(*end && isalnum(*end)) ++end;
        q=end;
        while(*q)
        {
            const unsigned char *qe,*a,*b;
            while(*q && !isalnum(*q)) ++q;
            if(!*q) break;
            qe=q; while(*qe && isalnum(*qe)) ++qe;
            if((size_t)(end-p)==(size_t)(qe-q))
            {
                a=p; b=q;
                while(a<end && tolower(*a)==tolower(*b)) {++a; ++b;}
                if(a==end) return false;
            }
            q=qe;
        }
        p=end;
    }
    return true;
}
static void TestBF29BF31BF32BF34(const char *root)
{
    Floppy144RunState run,loaded,legacy;
    uint8_t payload[FLOPPY144_SAVE_PAYLOAD_V6_SIZE];
    uint8_t old_payload[FLOPPY144_SAVE_PAYLOAD_V5_SIZE];
    char path[F144_PLATFORM_PATH_CAPACITY];
    uint32_t ci;
    uint32_t generated_total=0U,authored_total=0U;
    const uint32_t seeds[]={0U,1U,144U,0xFFFFFFFFU};
    Floppy144RunStateReset(&run);
    Expect(run.difficulty==FLOPPY144_DIFFICULTY_NORMAL,
        "BF32 zero-valued legacy state defaults to NORMAL");
    Expect(strcmp(Floppy144DifficultyName(FLOPPY144_DIFFICULTY_NORMAL),"NORMAL")==0 &&
           strcmp(Floppy144DifficultyName(FLOPPY144_DIFFICULTY_EASY),"EASY")==0 &&
           strcmp(Floppy144DifficultyName(FLOPPY144_DIFFICULTY_REALLY_SUPER_EASY),
              "REALLY SUPER EASY (I MEAN REALLY STUPIDLY EASY)")==0,
           "BF31 exact full difficulty names");
    Floppy144RunStateBegin(&run,1234U);
    for(ci=0U;ci<FLOPPY144_COLLECTION_COUNT;++ci)
    {
        Floppy144CollectionId collection=(Floppy144CollectionId)ci;
        const Floppy144CollectionDefinition *def=Floppy144CollectionGet(collection);
        uint32_t count=def->catalogue.record_count;
        uint32_t generated=0U,authored=0U,i,seed_i;
        uint64_t authored_bytes=0U,generated_bytes=0U,total=0U;
        for(i=0U;i<count;++i)
        {
            bool is_authored=Floppy144DocumentGet(collection,i)!=NULL;
            if(is_authored) ++authored;
            else ++generated;
            for(seed_i=0U;seed_i<(uint32_t)(sizeof(seeds)/sizeof(seeds[0]));++seed_i)
            {
                char id1[24],id2[24],title1[48],title2[48];
                Floppy144CatalogueBuildRecordForSeed(collection,i,seeds[seed_i],
                    id1,sizeof(id1),title1,sizeof(title1));
                Floppy144CatalogueBuildRecordForSeed(collection,i,seeds[seed_i],
                    id2,sizeof(id2),title2,sizeof(title2));
                Expect(strcmp(id1,id2)==0 && strcmp(title1,title2)==0,
                    "BF29 repeat generation is seeded deterministic");
                if(!is_authored)
                    Expect(TestTitleUniqueWords(title1),
                        "BF29 complete generated title has no duplicate words");
            }
            run.difficulty=FLOPPY144_DIFFICULTY_NORMAL;
            Expect(Floppy144CatalogueDocumentSizeBytes(&run,collection,i)==0U,
                "BF31 NORMAL hides all size data");
            run.difficulty=FLOPPY144_DIFFICULTY_EASY;
            {
                uint32_t bytes=Floppy144CatalogueDocumentSizeBytes(&run,collection,i);
                Expect(bytes>0U,"BF31 EASY shows authored and generated bytes");
                total+=bytes;
                if(is_authored)authored_bytes+=bytes;
                else generated_bytes+=bytes;
            }
        }
        Expect(total==(uint64_t)def->size_kb*1024U || count==0U,
            "BF31 EASY exact collection byte sum");
        if(authored>0U && generated>0U)
            Expect(authored_bytes*generated > generated_bytes*authored,
                "BF31 authored per-record average exceeds generated average");
        run.difficulty=FLOPPY144_DIFFICULTY_REALLY_SUPER_EASY;
        total=0U;
        for(i=0U;i<count;++i)
        {
            uint32_t bytes=Floppy144CatalogueDocumentSizeBytes(&run,collection,i);
            if(Floppy144DocumentGet(collection,i)!=NULL)
                Expect(bytes==0U,"BF31 easiest hides authored sizes");
            else
            {
                Expect(bytes>0U,"BF31 easiest shows generated sizes");
                total+=bytes;
            }
        }
        if(generated>0U)
            Expect(total==(uint64_t)def->size_kb*1024U,
                "BF31 easiest generated-only allocations sum precisely");
        generated_total+=generated;authored_total+=authored;
    }
    Expect(generated_total==1401U && authored_total==170U,
        "BF29 all 1571 current catalogue records were exercised");
    run.difficulty=FLOPPY144_DIFFICULTY_EASY;
    Expect(Floppy144PersistenceEncodeRunState(&run,payload,sizeof(payload)) &&
           Floppy144PersistenceDecodeRunState(&loaded,payload,sizeof(payload)) &&
           loaded.difficulty==FLOPPY144_DIFFICULTY_EASY,
        "BF32 V6 EASY encode and decode");
    run.difficulty=FLOPPY144_DIFFICULTY_REALLY_SUPER_EASY;
    Expect(TestJoinPath(root,"bf31-difficulty.sav",path,(uint32_t)sizeof(path)) &&
           Floppy144PersistenceSaveRunState(path,&run) &&
           Floppy144PersistenceLoadRunState(path,&loaded) &&
           loaded.difficulty==FLOPPY144_DIFFICULTY_REALLY_SUPER_EASY,
        "BF32 V6 save and reload preserves easiest mode");
    Expect(Floppy144PersistenceEncodeRunState(&run,old_payload,sizeof(old_payload)) &&
           Floppy144PersistenceDecodeRunState(&legacy,old_payload,sizeof(old_payload)) &&
           legacy.difficulty==FLOPPY144_DIFFICULTY_NORMAL,
        "BF32 V5 without difficulty migrates to NORMAL");
    run.difficulty=FLOPPY144_DIFFICULTY_NORMAL;
    Expect(Floppy144PersistenceEncodeRunState(&run,payload,sizeof(payload)) &&
           Floppy144PersistenceDecodeRunState(&loaded,payload,sizeof(payload)) &&
           loaded.difficulty==FLOPPY144_DIFFICULTY_NORMAL,
        "BF32 NORMAL remains a separately selectable mode");
    puts("BF29/BF31/BF32/BF34: all generated titles, three allocations, V5/V6 migration PASS");
}

static void TestBF19InspectionSettings(const char *root)
{
    Floppy144Settings settings,loaded;
    uint8_t bytes[FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE];
    char path[F144_PLATFORM_PATH_CAPACITY];
    Floppy144SettingsReset(&settings);
    Expect(settings.show_inspection_indicators==0U,
        "BF19 default setting OFF");
    Expect(Floppy144PersistenceEncodeSettings(
        &settings,bytes,sizeof(bytes))&&bytes[5]==0U,
        "BF19 old layout zero reserved byte is OFF");
    Expect(Floppy144PersistenceDecodeSettings(
        &loaded,bytes,sizeof(bytes))&&loaded.show_inspection_indicators==0U,
        "BF19 legacy settings decode OFF");
    Expect(Floppy144SettingsSetInspectionIndicators(&settings,true),
        "BF19 setting can switch on");
    Expect(Floppy144PersistenceEncodeSettings(
        &settings,bytes,sizeof(bytes))&&bytes[5]==1U &&
        Floppy144PersistenceDecodeSettings(&loaded,bytes,sizeof(bytes)) &&
        loaded.show_inspection_indicators==1U,
        "BF19 settings encode/decode ON");
    Expect(TestJoinPath(root,"bf19-settings.dat",path,
        (uint32_t)sizeof(path)) &&
        Floppy144PersistenceSaveSettings(path,&settings) &&
        Floppy144PersistenceLoadSettings(path,&loaded) &&
        loaded.show_inspection_indicators==1U,
        "BF19 settings restart retains ON");
    Expect(Floppy144SettingsSetInspectionIndicators(&loaded,false) &&
        Floppy144PersistenceSaveSettings(path,&loaded) &&
        Floppy144PersistenceLoadSettings(path,&settings) &&
        settings.show_inspection_indicators==0U,
        "BF19 settings restart retains OFF");
    bytes[5]=2U;
    Expect(Floppy144PersistenceDecodeSettings(
        &loaded,bytes,sizeof(bytes)) &&
        loaded.show_inspection_indicators==0U && loaded.dirty==1U,
        "BF19 malformed toggle normalizes to OFF");
}


/* BF33: capture actual secret-office framebuffer, including the same GDR
 * procedural logo renderer as the standard Credits. */
static void CaptureGreyDoorStudioMark(void)
{
    const char *dir=getenv("F144_BF33_CAPTURE_DIR");
    static uint32_t pixels[640U*360U];
    Floppy144Surface surface={pixels,640U,360U};
    Floppy144GreyEncounter scene;
    char path[512];
    FILE *out;
    uint32_t x,y;
    if(dir==NULL||dir[0]=='\0')return;
    memset(&scene,0,sizeof(scene));
    scene.phase=(uint8_t)FLOPPY144_GREY_EXPLORE;
    scene.local_x=115;scene.local_y=264;
    scene.body_style=FLOPPY144_OPERATOR_BODY_STYLE_A;
    Floppy144PlayerVisualReset(&scene.player_visual);
    Floppy144GreyEncounterDraw(&surface,&scene);
    (void)snprintf(path,sizeof(path),"%s/bf33_grey_door.ppm",dir);
    (void)fopen_s(&out,path,"wb");
    Expect(out!=NULL,"BF33 Grey Door screenshot opens");
    if(out==NULL)return;
    (void)fprintf(out,"P6\n640 360\n255\n");
    for(y=0U;y<360U;++y)for(x=0U;x<640U;++x)
    {
        uint32_t pixel=pixels[y*640U+x];
        (void)fputc((int)((pixel>>16U)&255U),out);
        (void)fputc((int)((pixel>>8U)&255U),out);
        (void)fputc((int)(pixel&255U),out);
    }
    (void)fclose(out);
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
    TestDifferentWorkingDirectory(root);
    TestGreyDoorDiscovery(root);
    TestGreyDoorPlacement(root);
    TestGreyEncounter(root);
    CaptureGreyDoorStudioMark();
    TestGreyDoorOneShotLifecycle(root);
    TestStage4GCompleteJourney(root);
    TestStage4HInspection(root);
    TestBF18DocumentNotebook(root);
    TestBF29BF31BF32BF34(root);
    TestBF19InspectionSettings(root);

    if(failures!=0)
    {
        printf("STAGE 4 PERSISTENCE PATH TESTS: FAIL (%d)\n",failures);
        return 1;
    }

    printf("STAGE 4 PERSISTENCE PATH TESTS: PASS\n");
    return 0;
}
