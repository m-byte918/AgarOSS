# Changelog

### v3.7
- Fixed invalid `UpdateNodes` packet being sent and added `writeStrNull()` method to Buffer class, thanks to @B0RYS :)
- Reorganized where files are placed
- Added variadic arguments to logging methods
- Refactored entity class
- Added entity acceleration and deceleration
- Added (experimental) playercell and virus splitting
- Added (experimental) ability to eject mass
- Added (experimental) method for collision resolution between cells
- Added border check for `Entitity::setPosition()`
- Fixed playercells moving too fast
- Now using pre-loaded configs rather than loading from json
- Switched to using `shared_ptr` for entities
- Added a few extra settings
- Refactored namespace `map`
- Minor tweaks to visible node updating
- Reformatted and tweaked `playerlist()` and `spawn()` commands
- Moved and renamed `Position` class from Utils.cpp/hpp to `Vector2` in Vector2.cpp/hpp
- Added `compile.sh` and `compile.bat` files to aid in compiling this project
- Added `Changelog.md`
---
### v1.8
- Rename `size` to `radius`
- Added and renamed some configs
- Fixed player viewbox center
- Removed and changed some methods from utilities
- Refactored `Commands` class to improved how commands are parsed
- Refactored `Entity` class
- Added flags for each entity type along with `canEat`, and `avoidSpawningOn` variables for efficient consumption and safe spawning
- Refactored map to comply with new entity flags