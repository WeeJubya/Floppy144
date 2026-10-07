/*
 * FLOPPY//144 Stage 4B Win32 single-instance regression.
 *
 * This test uses a unique synthetic profile root and launches a second copy of
 * itself. It never acquires the production user's AppData mutex and never
 * touches production persistence.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_win32_single_instance.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_PATH_CAPACITY 1024U

static int failures;

static void Expect(bool condition,const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n",label);
    }
}

static bool TestJoin(
    const char *root,
    const char *leaf,
    char *path,
    uint32_t capacity
)
{
    int written;

    if(root==NULL||leaf==NULL||path==NULL||capacity==0U)
    {
        return false;
    }

    written=snprintf(path,capacity,"%s\\%s",root,leaf);

    if(written<0||(uint32_t)written>=capacity)
    {
        path[0]='\0';
        return false;
    }

    return true;
}

static bool TestWriteSentinel(const char *path)
{
    static const char payload[]="FLOPPY144-SINGLE-INSTANCE-SENTINEL";
    FILE *file=NULL;
    size_t written;

    if(fopen_s(&file,path,"wb")!=0||file==NULL)
    {
        return false;
    }

    written=fwrite(payload,1U,sizeof(payload),file);

    return
        fclose(file)==0 &&
        written==sizeof(payload);
}

static bool TestSentinelUnchanged(const char *path)
{
    static const char payload[]="FLOPPY144-SINGLE-INSTANCE-SENTINEL";
    char buffer[sizeof(payload)];
    FILE *file=NULL;
    size_t read;
    int trailing;

    memset(buffer,0,sizeof(buffer));

    if(fopen_s(&file,path,"rb")!=0||file==NULL)
    {
        return false;
    }

    read=fread(buffer,1U,sizeof(buffer),file);
    trailing=fgetc(file);

    if(fclose(file)!=0)
    {
        return false;
    }

    return
        read==sizeof(payload) &&
        trailing==EOF &&
        memcmp(buffer,payload,sizeof(payload))==0;
}

static DWORD TestRunProbeChild(
    const char *profile_root
)
{
    char executable[TEST_PATH_CAPACITY];
    char command_line[TEST_PATH_CAPACITY*2U];
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    DWORD exit_code=0xFFFFFFFFU;
    int written;

    if(
        profile_root==NULL ||
        GetModuleFileNameA(
            NULL,
            executable,
            (DWORD)sizeof(executable)
        )==0U
    )
    {
        return 0xFFFFFFFFU;
    }

    if(!SetEnvironmentVariableA("F144_TEST_PROFILE_ROOT",profile_root))
    {
        return 0xFFFFFFFFU;
    }

    written=snprintf(
        command_line,
        sizeof(command_line),
        "\"%s\" --probe",
        executable
    );

    if(written<0||(size_t)written>=sizeof(command_line))
    {
        return 0xFFFFFFFFU;
    }

    memset(&startup,0,sizeof(startup));
    memset(&process,0,sizeof(process));
    startup.cb=sizeof(startup);

    if(
        !CreateProcessA(
            NULL,
            command_line,
            NULL,
            NULL,
            FALSE,
            0U,
            NULL,
            NULL,
            &startup,
            &process
        )
    )
    {
        return 0xFFFFFFFFU;
    }

    (void)WaitForSingleObject(
        process.hProcess,
        INFINITE
    );

    (void)GetExitCodeProcess(
        process.hProcess,
        &exit_code
    );

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    return exit_code;
}

static int TestProbeMode(void)
{
    char profile_root[TEST_PATH_CAPACITY];
    DWORD length;
    F144Win32SingleInstance instance={0};
    F144Win32SingleInstanceResult result;

    length=GetEnvironmentVariableA(
        "F144_TEST_PROFILE_ROOT",
        profile_root,
        (DWORD)sizeof(profile_root)
    );

    if(
        length==0U ||
        length>=(DWORD)sizeof(profile_root)
    )
    {
        return 20;
    }

    result=f144Win32SingleInstanceAcquireForProfileRoot(
        &instance,
        profile_root
    );

    if(result==F144_WIN32_SINGLE_INSTANCE_ALREADY_RUNNING)
    {
        return 7;
    }

    if(result!=F144_WIN32_SINGLE_INSTANCE_ACQUIRED)
    {
        return 21;
    }

    f144Win32SingleInstanceRelease(
        &instance
    );

    return 0;
}

int main(int argc,char **argv)
{
    char temp_root[TEST_PATH_CAPACITY];
    char test_root[TEST_PATH_CAPACITY];
    char profile_one[TEST_PATH_CAPACITY];
    char profile_two[TEST_PATH_CAPACITY];
    char sentinel[TEST_PATH_CAPACITY];
    DWORD temp_length;
    F144Win32SingleInstance first={0};
    F144Win32SingleInstance duplicate={0};
    F144Win32SingleInstance other_profile={0};
    F144Win32SingleInstance reacquired={0};

    if(argc>1&&strcmp(argv[1],"--probe")==0)
    {
        return TestProbeMode();
    }

    temp_length=GetTempPathA(
        (DWORD)sizeof(temp_root),
        temp_root
    );

    Expect(
        temp_length!=0U &&
        temp_length<(DWORD)sizeof(temp_root),
        "temporary root resolves"
    );

    if(failures!=0)
    {
        return 1;
    }

    {
        int written=snprintf(
            test_root,
            sizeof(test_root),
            "%sFloppy144-S4B06-%lu",
            temp_root,
            (unsigned long)GetCurrentProcessId()
        );

        Expect(
            written>0 &&
            (size_t)written<sizeof(test_root),
            "isolated test root builds"
        );
    }

    if(
        !CreateDirectoryA(test_root,NULL) &&
        GetLastError()!=ERROR_ALREADY_EXISTS
    )
    {
        Expect(false,"isolated test root creates");
    }

    Expect(
        TestJoin(
            test_root,
            "profile-one",
            profile_one,
            (uint32_t)sizeof(profile_one)
        ),
        "first profile environment builds"
    );
    Expect(
        TestJoin(
            test_root,
            "profile-two",
            profile_two,
            (uint32_t)sizeof(profile_two)
        ),
        "second profile environment builds"
    );
    Expect(
        TestJoin(
            test_root,
            "floppy144_profile.dat",
            sentinel,
            (uint32_t)sizeof(sentinel)
        ),
        "sentinel profile path builds"
    );
    Expect(
        TestWriteSentinel(sentinel),
        "sentinel profile file writes"
    );

    Expect(
        f144Win32SingleInstanceAcquireForProfileRoot(
            &first,
            profile_one
        )==F144_WIN32_SINGLE_INSTANCE_ACQUIRED,
        "first instance acquires ownership"
    );

    Expect(
        f144Win32SingleInstanceAcquireForProfileRoot(
            &duplicate,
            profile_one
        )==F144_WIN32_SINGLE_INSTANCE_ALREADY_RUNNING,
        "same-process duplicate is refused"
    );

    Expect(
        TestRunProbeChild(profile_one)==7U,
        "second process is refused while first instance owns profile"
    );

    Expect(
        TestSentinelUnchanged(sentinel),
        "refused instance does not alter persistent sentinel"
    );

    Expect(
        f144Win32SingleInstanceAcquireForProfileRoot(
            &other_profile,
            profile_two
        )==F144_WIN32_SINGLE_INSTANCE_ACQUIRED,
        "different profile environment can acquire independently"
    );

    f144Win32SingleInstanceRelease(&other_profile);
    f144Win32SingleInstanceRelease(&duplicate);
    f144Win32SingleInstanceRelease(&first);

    Expect(
        first.handle==NULL,
        "clean shutdown clears ownership handle"
    );

    Expect(
        TestRunProbeChild(profile_one)==0U,
        "subsequent process succeeds after clean release"
    );

    Expect(
        f144Win32SingleInstanceAcquireForProfileRoot(
            &reacquired,
            profile_one
        )==F144_WIN32_SINGLE_INSTANCE_ACQUIRED,
        "ownership can be reacquired after later process exits"
    );

    f144Win32SingleInstanceRelease(&reacquired);
    f144Win32SingleInstanceRelease(&reacquired);

    Expect(
        TestSentinelUnchanged(sentinel),
        "single-instance lifecycle never modifies persistent sentinel"
    );

    (void)DeleteFileA(sentinel);
    (void)RemoveDirectoryA(test_root);

    if(failures!=0)
    {
        printf(
            "STAGE 4 SINGLE-INSTANCE TESTS: FAIL (%d)\n",
            failures
        );
        return 1;
    }

    printf("STAGE 4 SINGLE-INSTANCE TESTS: PASS\n");
    return 0;
}
