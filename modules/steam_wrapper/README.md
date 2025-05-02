# SteamWrapper - Steam SDK Dynamic integration

This module dynamically loads and wraps the Steam API and creates a singleton to interact with it.

It is designed to use the Steam API in a Godot project without needing to compile the Steamworks SDK into the engine because it dynamically loads the Steam API at runtime and provides a simple interface to interact with it.

If it is true that we do not need to compile the Steamworks SDK into the engine, however, to use the Steam API, you still need to have the Steamworks SDK binaries (.dll or .so) in the same directory as your Godot editor or in your game's executable directory.

You can download the Steamworks SDK from the Steamworks website.

Secondly, you need to set the `steam_appid`, which is the ID of your game on Steam. You can do this in Project Settings under the `Steam` category.

With those two steps done, you can start using the Steam API in your Godot project.

```gdscript
func _ready() -> void:
	if Steam.is_steam_initialized():
		print("Steam is initialized")
	else:
		print("Steam is not initialized")
		return

	# Get the current app ID
	var app_id = Steam.get_app_id()
	print("Current app ID: ", app_id)

	# Get the current user's Steam ID
	var steam_id = Steam.get_user_steam_id()
	print("Current user's Steam ID: ", steam_id)
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
