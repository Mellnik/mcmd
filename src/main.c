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

#include <stdlib.h>
#include "engine.h"
#include "natives.h"
#include "main.h"

logprintf_t logprintf;
extern void *pAMXFunctions;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	if(mcmd_engine_install() != 0)
	{
		logprintf("[mcmd] Plugin successfully loaded "PLUGIN_VERSION" (Compiled on "__DATE__", "__TIME__").");
		mcmd_engine_dump_memory();
	}
	else
	{
		logprintf("[mcmd] Engine failed to initalize. Aborting server start, memory may has been corrupt.");

		mcmd_engine_dump_memory();
		free(engine);

		exit(EXIT_FAILURE);
	}
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload(void)
{
	free(engine);

	logprintf("[mcmd] Plugin unloaded.");
}

AMX_NATIVE_INFO mcmd_natives[] =
{
	{"GetPlayerLastRequestTime", _mcmd_native_last_request_time},
	{NULL, NULL}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	int i;
	int amx_OprcIdx, amx_OgmiIdx;

	for(i = 0; i < PAWN_MAX_AMX; ++i)
	{
		if(engine->scripts[i] == NULL)
		{
			engine->scripts[i] = amx;
			break;
		}
	}

	if(engine->gamemode == NULL && 
		amx_FindPublic(amx, "OnPlayerRequestCommand", &amx_OprcIdx) == AMX_ERR_NONE && 
		amx_FindPublic(amx, "OnGameModeInit", &amx_OgmiIdx) == AMX_ERR_NONE)
	{
		engine->gamemode = amx;
		engine->gamemode_idx = amx_OprcIdx;
	}
	return amx_Register(amx, mcmd_natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	int i;
	if(amx == engine->gamemode)
	{
		engine->gamemode = NULL;
		engine->gamemode_idx = 0;
	}
	
	for(i = 0; i < PAWN_MAX_AMX; i++)
	{
		if(engine->scripts[i] == amx)
		{
			engine->scripts[i] = NULL;
			break;
		}
	}
	return AMX_ERR_NONE;
}
