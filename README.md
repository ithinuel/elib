elib
====

Embedded library from memory manager to hi-level services.
Everything is in the title.

some flags :  
[![Build Status](https://travis-ci.org/ithinuel/elib.svg?branch=master)](https://travis-ci.org/ithinuel/elib) [![Coverage Status](https://coveralls.io/repos/ithinuel/elib/badge.png?branch=master)](https://coveralls.io/r/ithinuel/elib?branch=master)


Folder hierarchy
================
- API : all public .h
- boards/&lt;board_name> : boards specific code, configuration file, linkerscript
- core : Core library implementation
  This code must be platform independant, and therefore care must be taken to endianness
- os/&lt;os_name> : Operating system binding code to the API.
- parts/&lt;part_name> : Part or part family specific core.
  Here go device drivers etc.
- third_party : Third party code.
  You can install here every third party library you want.

Development rules
=================
Apply Test Driven Development.


Plans
=====

Short term: 
 - MQTT client support.
 - MQTT-SN client support.

Long term:
 - Lua engine. 