# Stage 4C Operator Name Entry and Editing

## Scope

S4C-02 turns the existing persistent `operator_name` field into an editable
player identity without changing the profile file schema or recovery-save
format.

## Storage rules

The authoritative field remains:

```c
char operator_name[32];
```

Player-entered names therefore have:

- maximum stored length: **31 ASCII characters**;
- byte 31 reserved for the terminating NUL;
- mixed/lowercase storage preserved exactly;
- presentation may uppercase because the existing 5x7 font is uppercase-only.

The accepted new-entry character set is deliberately small and directly
renderable by the current font:

- A-Z / a-z;
- 0-9;
- space;
- apostrophe;
- hyphen;
- period.

At least one letter or digit is required. Empty, whitespace-only and
punctuation-only values cannot be confirmed.

Unsupported/control/non-ASCII input is ignored without modifying the edit
buffer. Input beyond 31 characters is ignored and leaves the buffer NUL
terminated.

## Compatibility

The profile persistence format is unchanged:

```text
version: 1
payload: 64 bytes
```

The existing decoder deliberately does not retroactively apply the new
player-entry validation. A legacy Stage 3/Stage 4 profile containing a
NUL-terminated historical name outside the new editor's character subset still
loads and migrates.

Only new calls to the player-facing setter/editor enforce the S4C-02 rules.

## First-time behaviour

Operator identity remains **optional for gameplay**.

This preserves the established Stage 3/S4C-01 behaviour in which a fresh
profile may be `UNASSIGNED` and still initiate a recovery.

When a player opens **OPERATOR PROFILE** with no stored name, the screen enters
`NEW OPERATOR SETUP` immediately:

- type through the platform-neutral text-input stream;
- Backspace deletes;
- Enter validates and saves;
- Escape cancels setup without assigning a name.

After cancellation the player remains on Profile and may leave normally.

## Editing an existing name

A named profile shows:

```text
ENTER  EDIT NAME
```

Enter begins editing with the existing name copied into a transient buffer.

The persistent profile is not changed while typing. Escape cancels the
transient edit and leaves the original name untouched.

On Enter:

1. the candidate is validated;
2. the current profile structure is copied;
3. `Floppy144DiscoveryProfileSetOperatorName()` updates the in-memory profile;
4. the existing profile persistence path is atomically saved;
5. on save failure, the original in-memory profile is restored and the edit
   remains open for retry/cancel.

An unchanged name simply closes the editor without an unnecessary write.

## Text-input route

The Win32 layer remains unchanged.

`WM_CHAR` is translated by the established Stage 4B adapter into:

```c
F144TextInputEvent
```

The game coordinator routes those codepoints to the portable
`Floppy144ProfileNameEditState`.

Printable keys which also have gameplay meanings, such as A, I, W and S, have
their logical actions consumed while name editing is active. Their characters
arrive only through the separate text event.

Backspace is likewise consumed as a logical action and applied when the text
event supplies `'\b'`. Enter uses the logical Confirm action and its trailing
carriage-return text event is ignored. Escape uses the logical Menu action to
cancel editing.

No new Win32 key codes or message parsing were added to Core/editor code.

## Profile identity versus recovery saves

The operator name remains part of `Floppy144DiscoveryProfile`, not
`Floppy144RunState`.

Manual saves, autosaves and reinstatement therefore do not own or replace
identity. Starting another recovery increments profile history but retains the
same operator name.

The portable accessor:

```c
Floppy144DiscoveryProfileOperatorName()
```

makes the persistent value available for later terminal personalisation without
requiring a platform-specific input dependency.

## Regression coverage

S4C-02 adds focused tests for:

- fresh optional setup;
- short names;
- mixed-case storage;
- spaces;
- apostrophe/hyphen/period;
- maximum 31-character input;
- NUL termination at capacity;
- repeated over-capacity input with a memory guard;
- editing an existing name;
- Backspace deletion;
- cancellation preserving the original;
- repeated editing;
- empty and whitespace-only confirmation;
- unsupported printable/control/non-ASCII input;
- name retention when another recovery begins;
- profile save/reload as an application-restart analogue;
- independence from manual/autosave reinstate state;
- legacy profile migration preserving a historical name outside the new editor
  subset;
- source audit proving the editor uses the Stage 4B text-input boundary.

The complete Stage 3 and Stage 4B suites remain part of CI.
