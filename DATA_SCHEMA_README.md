# Floppy//144 canonical game data

`floppy144_game_data.json` is the canonical content and world-data source.

Counts: {
  "collections": 35,
  "documents": 157,
  "budgeted_readable_documents": 153,
  "triggers": 50,
  "interactions": 40,
  "ambient_interactions": 16,
  "evidence": 23,
  "physical_items": 161,
  "rooms": 11,
  "furniture": 151,
  "fixtures": 91,
  "drawing_definitions": 24,
  "colours": 44,
  "budgeted_reconstruction_pool_percent": 150,
  "registered_reconstruction_weight_percent": 165
}

The JSON intentionally contains both parent fields and a redundant `relationships` edge list. The compiler validates stable IDs before emitting C definitions. Runtime JSON parsing is not required in the release executable.
