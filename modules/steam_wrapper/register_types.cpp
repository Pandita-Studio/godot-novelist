//===========================================================================//
// Steam Wrapper - register_types.cpp
//===========================================================================//
//
// Copyright (c) 2025-Current | edo0xff and Contributors (view contributors.md)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//===========================================================================//

#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "steam_wrapper.h"

static SteamWrapper *Steam_singleton = nullptr;

void initialize_steam_wrapper_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<SteamWrapper>();
    Steam_singleton = memnew(SteamWrapper);
    Engine::get_singleton()->add_singleton(Engine::Singleton("Steam", SteamWrapper::get_singleton()));
}

void uninitialize_steam_wrapper_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
    Engine::get_singleton()->remove_singleton("Steam");
    memdelete(Steam_singleton);
}