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

#include <time.h>

#include "engine.h"
#include "natives.h"
#include "main.h"

/* native GetPlayerLastRequestTime(playerid); */
cell AMX_NATIVE_CALL _mcmd_native_last_request_time(AMX *amx, cell *params)
{
	int playerid;

	PARAM_CHECK(1, "GetPlayerLastRequestTime");

	playerid = (int)params[1];

	if (playerid < 0 || playerid > 1000) {
		logprintf("[mcmd] Invalid playerid in GetPlayerLastRequestTime.");
		return 0;
	}

	if(engine_pawn->reqtime[playerid] == INVALID_REQ_TIME)
		return INVALID_REQ_TIME;

	return (cell)((int)time(NULL) - engine_pawn->reqtime[playerid]);
}