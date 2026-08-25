#pragma once

#include <stdint.h>

/*
 * Floppy//144 Notebook Entry Registry
 *
 * Notebook IDs identify persistent player knowledge recorded during one
 * recovery session.
 *
 * Entries are independent of their source. Evidence, documents and
 * capabilities may all cause Notebook entries to be recorded.
 *
 * Definition order becomes significant once save persistence relies on these
 * numeric IDs. New entries should then be appended rather than inserted.
 */

typedef enum Floppy144NotebookId
{
    #define FLOPPY144_NOTEBOOK(symbol) \
    FLOPPY144_NOTEBOOK_##symbol,

    #include "floppy144_notebook_entries.def"

    #undef FLOPPY144_NOTEBOOK

    FLOPPY144_NOTEBOOK_COUNT
}
Floppy144NotebookId;
