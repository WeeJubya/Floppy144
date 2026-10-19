# BF38: Notebook pagination and Document Ledger

The original RECOVERY NOTEBOOK displays the existing persisted notebook_order
history for evidence and findings. BF18 document_note_order entries appear on
the separate DOCUMENT LEDGER page, in first-open order, with unchanged prose.
The game-data registry, document catalogue and save payloads are unchanged.

Left/Right switches pages, Up/Down scrolls one displayed line, and Page
Up/Page Down retains the 12-line jump. Each page has a RAM-only scroll offset.
Reopening from the Site selects the original page without clearing either
offset. Starting/reinstating a session resets both scroll positions.

## Test and handover

- tools/test_bugfix38_notebook.ps1: headless C regression of empty and heavily
  populated pages, 192 document summaries, scrolling bounds, independent
  positions, newly opened documents, reopen/reset and run-state immutability.
- Existing BF18 CI retains authored-summary coverage, legacy persistence and
  save/reload testing, Stage 3A/3B tests, Release and executable-size gate.
- Interactive Windows checks still required: fonts, clipping and keyboard
  behaviour at each supported resolution.

Branch: development/Stage-4. Executable limit: 1,474,560 bytes.
Review HEAD and CI results before accepting as release-verified.
