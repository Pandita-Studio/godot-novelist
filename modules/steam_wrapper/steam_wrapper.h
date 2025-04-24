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

#ifndef STEAM_WRAPPER_H
#define STEAM_WRAPPER_H

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/object/ref_counted.h"
#include "core/os/os.h"

enum SteamAPIInitResult {
	SteamAPIInitResult_OK = 0,
	SteamAPIInitResult_FailedGeneric = 1,
	SteamAPIInitResult_NoSteamClient = 2,
	SteamAPIInitResult_VersionMismatch = 3,
};

// Minimal type signatures
typedef void ISteamUser;

// Init functions (see steam_api.h for signature reference)
typedef bool (*SteamAPI_InitFunction)();
typedef SteamAPIInitResult (*SteamAPI_InitFlatFunction)(char *r_err_msg);
typedef void (*SteamAPI_ShutdownFunction)();

// Flat API functions (see steam_api_flat.h for signature reference)
typedef ISteamUser *(*SteamAPI_SteamUser_Function)();
typedef uint64_t (*SteamAPI_ISteamUser_GetSteamID_Function)(ISteamUser *self);

class SteamWrapper : public Object {
	GDCLASS(SteamWrapper, Object);

	public:
		static SteamWrapper *get_singleton();

		SteamWrapper();
		~SteamWrapper();

		uint32_t get_app_id();
		bool is_steam_initialized();

		// Wrapped API functions
		uint64_t get_user_steam_id();

	protected:
		static SteamWrapper *singleton;
		static void _bind_methods();

	private:
		// Init functions pointers
		SteamAPI_InitFunction steam_init_function = nullptr;
		SteamAPI_InitFlatFunction steam_init_flat_function = nullptr;
		SteamAPI_ShutdownFunction steam_shutdown_function = nullptr;

		// Flat api functions pointers
		SteamAPI_SteamUser_Function steam_get_user_interface_function = nullptr;
		SteamAPI_ISteamUser_GetSteamID_Function steam_get_steamid_function = nullptr;

		// Interfaces and library pointers
		ISteamUser *steam_user_interface = nullptr;
		void *steam_library_handle = nullptr;

		bool steam_initialized = false;
		bool debug_enabled;
		uint32_t app_id;

		// Helpers
		void module_print(const String &s);
		String init_result_to_string(const SteamAPIInitResult r);
};

#endif // STEAM_WRAPPER_H
