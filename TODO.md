# Emulator TODOs

* Rename the project to ProtoNES.

  1. Although this started out as a experiment fork of emudore, barely any
     of the original emudore code is used anymore.
  2. Savestates are stored in protobufs.

* Cross-platform builds.

* Get rid of the current GUI shell in favor of my own ImGui classes.

* Refactor the various `DebugStuff` routines for cleaner separation between
  emulation functionality and debug visualization.

* Investigate adding scripting language support (Python, Lua or Scheme).

* Clean up the DebugConsole interface.

* Improve debugging interfaces

  1. Clean up cpu watchpoint stuff and make it mapper/bank aware.
  2. Custom memory dumps, e.g.:
     "Player X velocity: ${byte:0x70}"
  3. Investigate radare.
