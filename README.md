NateCommander
=================

About
---
This is a game engine and various game implementations, written by Nathan Schubkegel (nathan.schubkegel@gmail.com) from scratch using permissive free and open source licensing and libraries. It exists publicly to showcase the author's personal hobby.

The long term goal for this project is to produce a game that combines many sub-games with obvious and enjoyable interaction, so fellows at a LAN party can continue playing "together" even when someone gets bored. (Example: tired of losing at the CounterStrike sub-game? Switch to the F-Zero sub-game and race through the CounterStrike map to see if you can squish the people who kept shooting you!)

The short term goal for this project is to entertain the author, so he keeps working on it! For example:

* The engine is written in C because it's a relaxing diversion from C# and lacks the "you're not using your programming language to its full potential" angst of C++ development.
* SDL is used to minimize the cost to port to Android someday.
* Some stuff that should have been provided via a third-party library (like an XML parser, data structures, etc) are written/reinvented because they're fun to write!
* All dependencies are statically linked because a single-exe application is a fun bragging point.
* The sub-games are written in Lua because "always wearing C goggles while programming" stifles the author's ability to make progress. Heck - project work halted for a year after interest in "a game engine that performs memory allocations only at startup" resulted in some incomplete data structures and a dismal outlook for sub-game design.
* The physics engine (Tokamak) was chosen because it looked like it wouldn't work off-the-shelf (tons of VS warnings about uninitialized variables). Changing a thing is learning it! Fun!

Building
---
The source code builds with Visual Studio 2008 Professional (because later versions of VS just get bloatier)

Graphic models are built and exported as Collada files using Blender 2.71

Licensing
---
The contents of this repo, excluding any submodules / subrepos, are free and unencumbered software released into the public domain under The Unlicense. You have complete freedom to do anything you want with the software, for any purpose. Please refer to <http://unlicense.org/> .

The contents of the submodules / subrepos are copyrighted and licensed as described in each respectively.

* ccan - various licenses
* lua - MIT license
* directx - a EULA that doesn't really specify, but it's freely available for download from MSDN so we're going with that
* sdl - zlib license
* tokamak - BSD or zlib license
* crypto-algorithms - public domain