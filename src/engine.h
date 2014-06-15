/*
 * Copyright (C) 2014 Mellnik
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "config.h"
#include <SDK/plugin.h>

#define INVALID_REQ_TIME 0x7FFFFFFF

int mcmd_engine_install(void);
int _mcmd_engine_detour(int playerid, char *cmdtext);
void mcmd_engine_dump_memory(void);

struct mcmd_engine_t {
	mcmd_dword	start;
	mcmd_dword	fail;
	mcmd_dword	success;
	AMX*		gamemode;
	int			gamemode_idx;
	AMX*		scripts[PAWN_MAX_AMX];
	int			reqtime[PAWN_MAX_PLAYERS];
};

extern struct mcmd_engine_t *engine;

#endif