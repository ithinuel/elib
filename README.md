elib
====

Embedded library from memory manager to hi-level services.
Everything is in the title.

folder hierarchy
================
- API : all public .h
- boards/&lt;board_name> : boards specific code, configuration file, linkerscript
- core : Core library implementation
  This code must be platform independant, and therefore care must be taken to endianness
- os/&lt;os_name> : Operating system binding code to the API.
- parts/&lt;part_name> : Part or part family specific core.
  Here goes device drivers etc.
- third_party : Third party code.
  You can install here every third party library you want.
