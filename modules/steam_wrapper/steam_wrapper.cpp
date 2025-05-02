/**************************************************************************/
/*  steam_wrapper.cpp                                                     */
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

#include "steam_wrapper.h"

SteamWrapper *SteamWrapper::singleton = nullptr;

SteamWrapper *SteamWrapper::get_singleton() {
	return singleton;
}

SteamWrapper::SteamWrapper() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	debug_enabled = GLOBAL_DEF_BASIC("steam/settings/print_logs", true);
	app_id = GLOBAL_DEF_BASIC("steam/settings/app_id", 0);

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (app_id == 0) {
		module_print("If you want to use Steam API, you need set your app_id in project settings (steam > settings)");
		return;
	}

	OS::get_singleton()->set_environment("SteamAppId", itos(app_id));
	OS::get_singleton()->set_environment("SteamGameId", itos(app_id));

	module_print("Trying to load SteamAPI library");

	bool library_located = true;
	String path;
	if (OS::get_singleton()->has_feature("linuxbsd")) {
		path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("libsteam_api.so");
		if (!FileAccess::exists(path)) {
			path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("../lib").path_join("libsteam_api.so");
			if (!FileAccess::exists(path)) {
				library_located = false;
			}
		}
	} else if (OS::get_singleton()->has_feature("windows")) {
		if (OS::get_singleton()->has_feature("64")) {
			path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("steam_api64.dll");
		} else {
			path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("steam_api.dll");
		}
		if (!FileAccess::exists(path)) {
			library_located = false;
		}
	} else if (OS::get_singleton()->has_feature("macos")) {
		path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("libsteam_api.dylib");
		if (!FileAccess::exists(path)) {
			path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("../Frameworks").path_join("libsteam_api.dylib");
			if (!FileAccess::exists(path)) {
				library_located = false;
			}
		}
	} else {
		module_print("Platform not supported");
		return;
	}

	if (!library_located) {
		module_print("Cannot locate SteamAPI library, make sure that you placed the dll/so in the executable folder");
		return;
	}

	// Let's try to dynamically load the Steam API library
	Error err = OS::get_singleton()->open_dynamic_library(path, steam_library_handle);
	if (err != OK) {
		steam_library_handle = nullptr;
		module_print("Error loading SteamAPI library");
		return;
	}
	module_print("SteamAPI library loaded");

	// Let's try to load init/shutdown functions
	void *symbol_handle = nullptr;
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_InitFlat", symbol_handle, true); // Try new API, 1.59+.
	if (err != OK) {
		err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_Init", symbol_handle); // Try old API.
		if (err != OK) {
			module_print("Error loading SteamAPI_Init");
			return;
		}
		steam_init_function = (SteamAPI_Init_Function)symbol_handle;
	} else {
		steam_init_flat_function = (SteamAPI_InitFlat_Function)symbol_handle;
	}

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_Shutdown", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_Shutdown");
		return;
	}
	steam_shutdown_function = (SteamAPI_Shutdown_Function)symbol_handle;

	// Let's try to init SteamAPI
	if (steam_init_flat_function) {
		char err_msg[1024] = {};
		SteamAPIInitResult result = steam_init_flat_function(&err_msg[0]);
		steam_initialized = (result == SteamAPIInitResult_OK);
		module_print("Status: " + init_result_to_string(result));
	} else if (steam_init_function) {
		steam_initialized = steam_init_function();
	}

	module_print(steam_initialized ? "Init: OK" : "Init: FAIL");

	// After init, let's try to load some interfaces-related functions

	// SteamAPI_SteamUser_v023 can be loaded but SteamAPI_SteamUser dont, why?
	// ToDo: check for multiple versions for better compatibility
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_SteamUser_v023", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_SteamUser_v023");
		return;
	}
	steam_get_user_interface_function = (SteamAPI_SteamUser_Function)symbol_handle;

	// Let's try to load SteamUser interface
	steam_user_interface = steam_get_user_interface_function();

	if (!steam_user_interface) {
		module_print("SteamUser interface cannot be loaded");
		return;
	}

	// Flat functions
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_ISteamUser_GetSteamID", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_ISteamUser_GetSteamID");
		return;
	}
	steam_get_steamid_function = (SteamAPI_ISteamUser_GetSteamID_Function)symbol_handle;

	// Callbacks
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_GetHSteamUser", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_GetHSteamUser");
		return;
	}
	steam_get_huser_function = (SteamAPI_GetHSteamUser_Function)symbol_handle;

	h_user = steam_get_huser_function();

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_GetHSteamPipe", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_GetHSteamPipe");
		return;
	}
	steam_get_hpipe_function = (SteamAPI_GetHSteamPipe_Function)symbol_handle;

	h_pipe = steam_get_hpipe_function();

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_ManualDispatch_Init", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_ManualDispatch_Init");
		return;
	}
	steam_manual_dispatch_init_function = (SteamAPI_ManualDispatch_Init_Function)symbol_handle;

	steam_manual_dispatch_init_function();

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_ManualDispatch_RunFrame", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_ManualDispatch_RunFrame");
		return;
	}
	steam_manual_dispatch_run_frame_function = (SteamAPI_ManualDispatch_RunFrame_Function)symbol_handle;

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_ManualDispatch_GetNextCallback", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_ManualDispatch_GetNextCallback");
		return;
	}
	steam_manual_dispatch_get_next_callback_function = (SteamAPI_ManualDispatch_GetNextCallback_Function)symbol_handle;

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(steam_library_handle, "SteamAPI_ManualDispatch_FreeLastCallback", symbol_handle);
	if (err != OK) {
		module_print("Error loading SteamAPI_ManualDispatch_FreeLastCallback");
		return;
	}
	steam_manual_dispatch_free_last_callback_function = (SteamAPI_ManualDispatch_FreeLastCallback_Function)symbol_handle;
}

SteamWrapper::~SteamWrapper() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;

	// Clean up
	if (steam_initialized && steam_shutdown_function) {
		steam_shutdown_function();
	}
	if (steam_library_handle) {
		OS::get_singleton()->close_dynamic_library(steam_library_handle);
	}
}

// Wrapped API functions
uint64_t SteamWrapper::get_user_steam_id() {
	if (!steam_initialized || !steam_user_interface || !steam_get_steamid_function) {
		module_print("Cannot retrieve Steam user ID, interface not available");
		return 0;
	}
	return steam_get_steamid_function(steam_user_interface);
}

void SteamWrapper::run_callbacks() {
	if (!steam_initialized) {
		module_print("Cannot run callbacks, SteamAPI not initialized");
		return;
	}

	if (!steam_manual_dispatch_run_frame_function || !steam_manual_dispatch_get_next_callback_function) {
		return;
	};

	steam_manual_dispatch_run_frame_function(h_pipe);

	CallbackMsg_t callback_msg;

	while (steam_manual_dispatch_get_next_callback_function(h_pipe, &callback_msg)) {
		if (callback_msg.m_iCallback == GameOverlayActivated_t::k_iCallback) {
			GameOverlayActivated_t *overlay_callback = (GameOverlayActivated_t *)callback_msg.m_pubParam;
			bool active = overlay_callback->m_bActive != 0;
			emit_signal("game_overlay_toggled", active);
		}
		steam_manual_dispatch_free_last_callback_function(h_pipe);
	}
}

void SteamWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_app_id"), &SteamWrapper::get_app_id);
	ClassDB::bind_method(D_METHOD("is_steam_initialized"), &SteamWrapper::is_steam_initialized);
	ClassDB::bind_method(D_METHOD("get_steam_wrapper_version"), &SteamWrapper::get_steam_wrapper_version);
	ClassDB::bind_method(D_METHOD("get_steam_api_version"), &SteamWrapper::get_steam_api_version);
	ClassDB::bind_method(D_METHOD("get_user_steam_id"), &SteamWrapper::get_user_steam_id);
	ClassDB::bind_method(D_METHOD("run_callbacks"), &SteamWrapper::run_callbacks);

	ADD_SIGNAL(MethodInfo("game_overlay_toggled", PropertyInfo(Variant::BOOL, "active")));
}

// Helpers
void SteamWrapper::module_print(const String &s) {
	if (debug_enabled) {
		print_line("[Steam] " + s);
	}
}

String SteamWrapper::init_result_to_string(const SteamAPIInitResult r) {
	switch (r) {
		case SteamAPIInitResult_OK:
			return "OK";
		case SteamAPIInitResult_VersionMismatch:
			return "Version Mismatch";
		case SteamAPIInitResult_NoSteamClient:
			return "No Steam Client";
		default:
			return "Generic Fail";
	}
}
