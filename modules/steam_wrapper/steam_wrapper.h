/**************************************************************************/
/*  steam_wrapper.h                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/object/ref_counted.h"
#include "core/os/os.h"

#define STEAM_USER_INTERFACE_VERSION "SteamAPI_SteamUser_v023"
#define STEAM_USERSTATS_INTERFACE_VERSION "SteamAPI_SteamUserStats_v013"

// As we are loading the library dynamically, we need to define the minimal types signatures.
// See steam_api.h, steam_api_flat.h, steam_api_common.h, steam_api_internal.h and steam_api.json for reference.
// (Steam API headers are not included)
enum SteamAPIInitResult {
	SteamAPIInitResult_OK = 0,
	SteamAPIInitResult_FailedGeneric = 1,
	SteamAPIInitResult_NoSteamClient = 2,
	SteamAPIInitResult_VersionMismatch = 3,
};

typedef int32_t HSteamUser;
typedef int32_t HSteamPipe;
typedef uint32_t AppId_t;

// Callbacks structs signatures
struct CallbackMsg_t {
	HSteamUser m_hSteamUser;
	int m_iCallback;
	uint8_t *m_pubParam;
	int m_cubParam;
};

struct GameOverlayActivated_t {
	uint8_t m_bActive;
	bool m_bUserInitiated;
	AppId_t m_nAppID;
	uint32_t m_dwOverlayPID;
	static const int k_iCallback = 331;
};

// Init functions signatures
typedef bool (*SteamAPI_Init_Function)();
typedef SteamAPIInitResult (*SteamAPI_InitFlat_Function)(char *r_err_msg);
typedef void (*SteamAPI_Shutdown_Function)();

// Needed for callbacks manual dispatch
typedef HSteamUser (*SteamAPI_GetHSteamUser_Function)();
typedef HSteamPipe (*SteamAPI_GetHSteamPipe_Function)();

// Callbacks manual dispatch related functions signatures
typedef void (*SteamAPI_ManualDispatch_Init_Function)();
typedef void (*SteamAPI_ManualDispatch_RunFrame_Function)(HSteamPipe hSteamPipe);
typedef bool (*SteamAPI_ManualDispatch_GetNextCallback_Function)(HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg);
typedef void (*SteamAPI_ManualDispatch_FreeLastCallback_Function)(HSteamPipe hSteamPipe);

// Flat API functions signatures
typedef void ISteamUser;
typedef ISteamUser *(*SteamAPI_SteamUser_Function)();
typedef uint64_t (*SteamAPI_ISteamUser_GetSteamID_Function)(ISteamUser *self);

typedef void ISteamUserStats;
typedef ISteamUserStats *(*SteamAPI_SteamUserStats_Function)();
typedef bool (*SteamAPI_ISteamUserStats_GetAchievement)(ISteamUserStats *self, const char *pchName, bool *pbAchieved);
typedef bool (*SteamAPI_ISteamUserStats_SetAchievement)(ISteamUserStats *self, const char *pchName);
typedef bool (*SteamAPI_ISteamUserStats_ClearAchievement)(ISteamUserStats *self, const char *pchName);
typedef bool (*SteamAPI_ISteamUserStats_StoreStats)(ISteamUserStats *self);

class SteamWrapper : public Object {
	GDCLASS(SteamWrapper, Object);

public:
	static SteamWrapper *get_singleton();

	SteamWrapper();
	~SteamWrapper();

	inline uint32_t get_app_id() { return app_id; }
	inline bool is_steam_initialized() { return steam_initialized; }
	inline String get_steam_wrapper_version() { return steam_wrapper_version; }
	inline String get_steam_api_version() { return steam_api_version; }

	// Wrapped API functions
	uint64_t get_user_steam_id();

	bool get_achievement(const String &name);
	void set_achievement(const String &name);
	void clear_achievement(const String &name);

	// This function should be called every frame (on _physics_process() maybe?) to process callbacks
	void run_callbacks();

protected:
	static SteamWrapper *singleton;
	static void _bind_methods();

private:
	// Init functions pointers
	SteamAPI_Init_Function steam_init_function = nullptr;
	SteamAPI_InitFlat_Function steam_init_flat_function = nullptr;
	SteamAPI_Shutdown_Function steam_shutdown_function = nullptr;

	// Flat api functions pointers
	SteamAPI_SteamUser_Function steam_get_user_interface_function = nullptr;
	SteamAPI_ISteamUser_GetSteamID_Function steam_get_steamid_function = nullptr;

	SteamAPI_SteamUserStats_Function steam_get_user_stats_interface_function = nullptr;
	SteamAPI_ISteamUserStats_GetAchievement steam_get_achievement_function = nullptr;
	SteamAPI_ISteamUserStats_SetAchievement steam_set_achievement_function = nullptr;
	SteamAPI_ISteamUserStats_ClearAchievement steam_clear_achievement_function = nullptr;
	SteamAPI_ISteamUserStats_StoreStats steam_store_stats_function = nullptr;

	// Callbacks functions pointers
	SteamAPI_GetHSteamUser_Function steam_get_huser_function = nullptr;
	SteamAPI_GetHSteamPipe_Function steam_get_hpipe_function = nullptr;

	SteamAPI_ManualDispatch_Init_Function steam_manual_dispatch_init_function = nullptr;
	SteamAPI_ManualDispatch_RunFrame_Function steam_manual_dispatch_run_frame_function = nullptr;
	SteamAPI_ManualDispatch_GetNextCallback_Function steam_manual_dispatch_get_next_callback_function = nullptr;
	SteamAPI_ManualDispatch_FreeLastCallback_Function steam_manual_dispatch_free_last_callback_function = nullptr;

	// Interfaces, library and stuff pointers
	HSteamUser h_user;
	HSteamPipe h_pipe;
	ISteamUser *steam_user_interface = nullptr;
	ISteamUserStats *steam_user_stats_interface = nullptr;
	void *steam_library_handle = nullptr;

	// Some internal variables
	String steam_wrapper_version = "0.0.1";
	String steam_api_version = "1.62";
	bool steam_initialized = false;
	bool debug_enabled;
	uint32_t app_id;

	// Helpers
	void module_print(const String &s);
	String init_result_to_string(const SteamAPIInitResult r);
};
