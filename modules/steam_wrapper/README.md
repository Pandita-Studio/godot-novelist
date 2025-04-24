# SteamWrapper - Steam SDK Dynamic integration

This module dynamically loads and wraps the Steam API and creates a singleton to interact with it.

```gdscript
func _ready():
    print(Steam.is_steam_initialized()) #True if everything goes well!
```

> [!TIP]
> You don’t need to validate for singleton presence using something like `Engine.has_singleton("Steam")`, because the Steam singleton will be present on all platforms. However, it will only function on Linux, Windows, and macOS.

## Motivation

Since we are dynamically loading the Steam API, we get some advantages:

- We don't need to include Steam API headers during the build process.
- The Steam API will load only if the API distributables are available. This is optional, if the Steam libraries are not present, the wrapper won’t crash, it will simply notify that the Steam API is not available.
- Since we don't have to include Steam API headers or libraries during compilation, the build process becomes much easier, especially in CI environments. 

## To-Do

The API wrapper still needs further development. For now, it’s just a concept that dynamically loads the API, initializes the SteamUser interface, and loads some flat API functions. The full API still needs to be implemented.

## References

- GodotSteam: https://github.com/GodotSteam/GodotSteam
- Inspired by: https://github.com/GodotSteam/GodotSteam/pull/810
- Godot Engine: https://github.com/godotengine/godot/blob/master/main/steam_tracker.h
