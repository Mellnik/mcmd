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

// mcmd Engine 0.0.1
 
#if defined _mcmd_included
	#endinput
#endif
#define _mcmd_included

#define mcmd:%1(%2) \
	forward _ce%1(%2); \
	public _ce%1(%2)
	
#if !defined isnull
	#define isnull(%1) ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif
	
forward OnPlayerRequestCommand(playerid, cmdtext[], exists);

native GetPlayerLastRequestTime(playerid);