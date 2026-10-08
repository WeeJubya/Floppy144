/*
 * Stage 4E paperback generator. Exhaustive 156,160-combination layout audit,
 * deterministic golden vectors and safe output-buffer handling.
 */
#include "floppy144_paperback.h"
#include "floppy144_variation.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t failures;

static void Check(bool ok, const char *message)
{
    if(!ok)
    {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static bool ThreeReadableLines(const char *text)
{
    uint32_t lines = 1U;
    uint32_t length = 0U;
    const unsigned char *p = (const unsigned char *)text;
    unsigned char previous = 0U;

    if(text == NULL || text[0] == '\0')
        return false;

    while(*p != 0U)
    {
        if(*p == (unsigned char)'\n')
        {
            if(length == 0U || previous == (unsigned char)' ')
                return false;
            ++lines;
            length = 0U;
        }
        else
        {
            if(
                *p < 32U || *p > 126U ||
                (*p == (unsigned char)' ' &&
                 (previous == (unsigned char)' ' ||
                  previous == (unsigned char)'\n'))
            )
                return false;

            ++length;
            if(length > FLOPPY144_PAPERBACK_LINE_LIMIT)
                return false;
        }
        previous = *p;
        ++p;
    }
    return lines == 3U && length > 0U && previous != (unsigned char)' ';
}

static void TestKnownSeeds(void)
{
    char a[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
    char b[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
    char again[FLOPPY144_PAPERBACK_TEXT_CAPACITY];

    Check(
        Floppy144PaperbackGenerate(144U, a, (uint32_t)sizeof(a)) &&
        strcmp(a,
            "TITLE: SENIOR LINE MANAGER\n"
            "BY: CRISPIN QUIBBLE\n"
            "THE LAST PAGE IS A LEAVE REQUEST") == 0,
        "seed 144 selects known book title, author and annotation"
    );
    Check(
        Floppy144PaperbackGenerate(145U, b, (uint32_t)sizeof(b)) &&
        strcmp(b,
            "TITLE: HEARTBROKEN ARCHIVE CLERK\n"
            "BY: MILLICENT MUDDLE\n"
            "THE LAST PAGE IS A LEAVE REQUEST") == 0,
        "seed 145 selects a different known book and fictional author"
    );
    Check(strcmp(a,b) != 0, "known seeds produce visibly different covers");

    (void)Floppy144VariationRange(
        144U, "staff.crossword.pattern.v1", "P-074", 6U
    );
    (void)Floppy144VariationRange(
        144U, "takeaway.name.subject.v1", "P-330", 11U
    );
    Check(
        Floppy144PaperbackGenerate(144U, again, (uint32_t)sizeof(again)) &&
        strcmp(a, again) == 0,
        "unrelated feature variation cannot reshuffle the paperback"
    );
    printf("SEED 144 PAPERBACK:\n%s\n",a);
    printf("SEED 145 PAPERBACK:\n%s\n",b);
}

static void TestEveryPossibleCover(void)
{
    static const uint32_t widths[5U] = {8U,8U,8U,7U,8U};
    uint32_t form, left, right, given, family, note;
    uint32_t combinations=0U;
    uint32_t longest=0U;
    char cover[FLOPPY144_PAPERBACK_TEXT_CAPACITY];

    /*
     * Test every theoretical variant, including all maximal word lengths,
     * rather than assuming a broad sampled run-seed sweep reaches them.
     */
    for(form=0U;form<FLOPPY144_PAPERBACK_FORM_COUNT;++form)
    for(left=0U;left<widths[form];++left)
    for(right=0U;right<widths[form];++right)
    for(given=0U;given<FLOPPY144_PAPERBACK_AUTHOR_GIVEN_COUNT;++given)
    for(family=0U;family<FLOPPY144_PAPERBACK_AUTHOR_FAMILY_COUNT;++family)
    for(note=0U;note<FLOPPY144_PAPERBACK_NOTE_COUNT;++note)
    {
        const char *p;
        uint32_t length=0U;
        bool ok=Floppy144PaperbackCompose(
            form,left,right,given,family,note,
            cover,(uint32_t)sizeof(cover)
        );

        if(!ok || !ThreeReadableLines(cover))
        {
            Check(false,"every combination fits three lines with clean ASCII/spacing");
            return;
        }

        for(p=cover;*p!='\0';++p)
        {
            if(*p=='\n')
            {
                if(length>longest)
                    longest=length;
                length=0U;
            }
            else
                ++length;
        }
        if(length>longest)
            longest=length;
        ++combinations;
    }

    Check(
        combinations==156160U,
        "exhaustive pool enumeration covers 156,160 combinations"
    );
    Check(
        longest<=FLOPPY144_PAPERBACK_LINE_LIMIT,
        "all title/author/note lines remain within 54 bitmap glyphs"
    );
    printf("PAPERBACK EXHAUSTIVE: %u combinations; max line %u chars\n",
        combinations,longest);
}

static void TestSeedSweepAndBounds(void)
{
    uint32_t seed;
    char current[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
    char saved[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
    unsigned char guard[12];
    char tiny[1] = {'x'};
    uint32_t changes=0U;
    saved[0]='\0';

    for(seed=1U;seed<=10000U;++seed)
    {
        if(
            !Floppy144PaperbackGenerate(
                seed,current,(uint32_t)sizeof(current)
            ) || !ThreeReadableLines(current)
        )
        {
            Check(false,"10,000 saved-seed mappings produce readable covers");
            return;
        }

        if(seed>1U && strcmp(current,saved)!=0)
            ++changes;

        (void)snprintf(saved,sizeof(saved),"%s",current);
    }

    Check(changes>1000U,"diverse neighbouring seeds select distinct cover text");
    Check(
        !Floppy144PaperbackGenerate(144U,NULL,0U),
        "null output fails safely"
    );
    Check(
        !Floppy144PaperbackGenerate(144U,tiny,0U) && tiny[0]=='x',
        "zero-capacity output does not write"
    );
    Check(
        !Floppy144PaperbackGenerate(144U,tiny,1U) && tiny[0]=='\0',
        "too-small output is empty rather than truncated"
    );
    memset(guard,0x5AU,sizeof(guard));
    Check(
        !Floppy144PaperbackGenerate(144U,(char *)&guard[1],9U) &&
        guard[0]==0x5AU && guard[10]==0x5AU &&
        guard[11]==0x5AU && guard[1]=='\0',
        "failure preserves both buffer canaries and clears partial text"
    );
    Check(
        !Floppy144PaperbackCompose(5U,0U,0U,0U,0U,0U,
            current,(uint32_t)sizeof(current)) &&
        current[0]=='\0',
        "invalid form never produces a malformed title"
    );
    Check(
        !Floppy144PaperbackCompose(0U,8U,0U,0U,0U,0U,
            current,(uint32_t)sizeof(current)) &&
        current[0]=='\0',
        "invalid fragment index never reads beyond the word pool"
    );
}

int main(void)
{
    TestKnownSeeds();
    TestEveryPossibleCover();
    TestSeedSweepAndBounds();

    if(failures!=0U)
    {
        fprintf(stderr,"STAGE 4E PAPERBACK TESTS: FAIL (%u)\n",failures);
        return 1;
    }
    puts("STAGE 4E PAPERBACK TESTS: PASS");
    return 0;
}
