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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implie
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"
#include "memory.h"
#include "main.h"

struct mcmd_engine_t *engine;

#if defined __WIN32__ || defined _WIN32 || defined WIN32
_declspec(naked) void _mcmd_opct_jmp_hook(void) // we don't want a prologue here therefore naked
{
	_asm // EBX = cmdtext, EBP = playerid
	{
		push	ebx
		push	ebp
		call	_mcmd_engine_detour
		pop		ebp
		pop		ebx
		mov		edx, dword ptr [engine]
		test	eax, eax
		je		noproc
		mov		eax, [edx+08h] ; engine->success
		jmp		eax
	noproc:
		mov		eax, [edx+04h] ; engine->fail
		jmp		eax
	}
}
#else
void _mcmd_opct_jmp_hook(void)
{
	__asm__ volatile( // FUCK AT&T LOL !!!!!!!!
		"movl	-0x14(%%ebp),%%edx\n\t"
		"push	%%edx\n\t"
		"movl	0xC(%%ebp),%%ecx\n\t"
		"push	%%ecx\n\t"
		"call	_mcmd_engine_detour\n\t"
		"pop	%%ecx\n\t"
		"pop	%%edx\n\t"
		"movl	%[engine],%%edx\n\t"
		"test	%%eax,%%eax\n\t"
		"je		NOPROC%=\n\t"
		"movl	0x8(%%edx),%%eax\n\t"
		"jmp	*%%eax\n\t"
		"NOPROC%=:\n\t"
		"movl	0x4(%%esi),%%eax\n\t"
		"jmp	*%%eax"
		: 
		: [engine] "m" (engine) 
		: );	
}
#endif

int mcmd_engine_install(void)
{
	mcmd_dword protback, protdummy, distance, len, i;

	engine = (struct mcmd_engine_t *)malloc(sizeof(struct mcmd_engine_t));

	if(engine == NULL)
		return 0;

#if defined __WIN32__ || defined _WIN32 || defined WIN32
	// Get the address of where we hook in
	engine->start = mcmd_memory_scan("\x8B\x04\xB7\x85\xC0\x74\x63", "xxxxxxx");
	// Get the address of fail jump (if we want "SERVER: Unknown Command")
	engine->fail = mcmd_memory_scan("\x83\xFE\x10\x7C\x90\x8B\x44\x24\x10\x5F\x5E", "xxxxxxxxxxx") + 0x5;
	// Get the address of where we going to jump upon mcmd done
	engine->success = mcmd_memory_scan("\x83\xC4\x08\xC2\x08\x00\x5F", "xxxxxxx") + 0x6;
#else // 0x80A0A80 function start, 0.3z-R2-2
	// 0x80A0A93
	engine->start = mcmd_memory_scan("\x8B\x7D\x00\x89\xF3\xEB\x00\x8D\xB6\x00\x00\x00\x00\x83\xC3\x00\x8D\x46\x00\x39\xC3\x0F\x8F\x00\x00\x00\x00\x8B\x13\x85\xD2\x74\x00\x89\x14\x24\x8D\x45\x00\x89\x44\x24\x00\xB8\x00\x00\x00\x00\x89\x44\x24\x00\xE8\x00\x00\x00\x00\x85\xC0\x75\x00\x89\x7C\x24\x00\x31\xC0\x31\xC9\x89\x44\x24\x00", "xx?xxx?xxxxxxxx?xx?xxxx????xxxxx?xxxxx?xxx?x????xxx?x????xxx?xxx?xxxxxxx?");
	// 0x80A0B44
	engine->fail = mcmd_memory_scan("\x85\xD2\x0F\x84\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xEB\x00\x8B\x45\x00", "xxxx????x????x?xx?") + 0xF;
	// 0x80A0B3D
	engine->success = engine->fail - 0x7;
#endif

	if(engine->start <= 0 || engine->fail <= 0 || engine->success <= 0)
		return 0;

#if defined __WIN32__ || defined _WIN32 || defined WIN32
	len = engine->fail - engine->start;
#else
	len = engine->success - engine->start; // Success code comes before fail mov on linux
#endif
	
	// Calculate distance to our detour, -0x5 for jmp size
	distance = ((mcmd_dword)_mcmd_opct_jmp_hook - engine->start) - 0x5;

	// Make memory writeable
#if defined __WIN32__ || defined _WIN32 || defined WIN32
	VirtualProtect((mcmd_byte *)engine->start, len, PAGE_EXECUTE_READWRITE, &protback);
#else // stolen from sscanf by Y_Less
	size_t 
		iPageSize = getpagesize(),
		iAddr = (((size_t)engine->start / iPageSize) * iPageSize),
		iCount = ((size_t)len / iPageSize) * iPageSize + iPageSize * 2;
	mprotect((void *)iAddr, (size_t)iCount, PROT_EXEC | PROT_READ | PROT_WRITE);
#endif
	// Write jmp + our address
	*(mcmd_byte *)engine->start = 0xE9;
	*(mcmd_dword *)(engine->start + 0x1) = distance;

	// nop code after our jmp to end address
	for(i = 0x5; i < len; ++i)
		*(mcmd_byte *)(engine->start + i) = 0x90;

	// Be nice and restore old permission, TODO: restore on linux too?
#if defined __WIN32__ || defined _WIN32 || defined WIN32
	VirtualProtect((mcmd_byte *)engine->start, len, protback, &protdummy);
#endif

	// Just make sure everything's reset
	engine->gamemode = NULL;
	engine->gamemode_idx = 0;

	for(i = 0; i < PAWN_MAX_AMX; ++i)
		engine->scripts[i] = NULL;

	for(i = 0; i < PAWN_MAX_PLAYERS; ++i)
		engine->reqtime[i] = INVALID_REQ_TIME;
	return 1;
}

int _mcmd_engine_detour(int playerid, char *cmdtext)
{
	char	cmd[PAWN_MAX_FUNC_SIZE] = PAWN_CMD_PREFIX;
	AMX		*cmd_amx = NULL;
	int		cmd_idx = 0;
	int		i;
	int		pos = 0;
	cell	amx_rel = 0;
	cell	amx_ret = 0;

	while(cmdtext[++pos] != ' ' && pos < PAWN_MAX_FUNC_SIZE - 3)
		cmd[pos + 2] = tolower(cmdtext[pos]);

	if(cmdtext[pos++] != ' ') //loop above stopped because command name too long
	{
		//go to position after space char
		while(cmdtext[pos] != ' ' && cmdtext[pos] != '\0')
			pos++;

		if(cmdtext[pos] != '\0')
			pos++;
	}

	for(i = 0; i < PAWN_MAX_AMX && engine->scripts[i] != NULL; ++i)
	{
		if(amx_FindPublic(engine->scripts[i], cmd, &cmd_idx) == AMX_ERR_NONE)
		{
			cmd_amx = engine->scripts[i];
			break;
		}
	}

	engine->reqtime[playerid] = (int)time(NULL);

	if(engine->gamemode != NULL)
	{
		amx_Push(engine->gamemode, cmd_amx != NULL);
		amx_PushString(engine->gamemode, &amx_rel, NULL, cmdtext, 0, 0);
		amx_Push(engine->gamemode, playerid);

		amx_Exec(engine->gamemode, &amx_ret, engine->gamemode_idx);
		amx_Release(engine->gamemode, amx_rel);

		if(amx_ret && cmd_amx != NULL)
			goto execcmd;

		return 1;
	}

	if(cmd_amx == NULL) // if no callback + command doesn't exist: return "SERVER: Unknown Command"
		return 0;

execcmd:
	// Final command call
	amx_PushString(cmd_amx, &amx_rel, NULL, cmdtext + pos, 0, 0);
	amx_Push(cmd_amx, playerid);

	amx_Exec(cmd_amx, &amx_ret, cmd_idx);
	amx_Release(cmd_amx, amx_rel);
	return 1;
}

void mcmd_engine_dump_memory(void)
{
	if(engine == NULL) 
		return;
		
	logprintf("[mcmd] Dump engine->start: 0x%X", engine->start);
	logprintf("[mcmd] Dump engine->fail: 0x%X", engine->fail);
	logprintf("[mcmd] Dump engine->success: 0x%X", engine->success);
	logprintf("[mcmd] Dump engine->gamemode: 0x%X", engine->gamemode);
	logprintf("[mcmd] Dump engine->gamemode_idx: %i", engine->gamemode_idx);
}

		/* intel ?
	__asm__ volatile(
		".intel_syntax noprefix\n\t"
		"mov	edx,[ebx-14h]\n\t"
		"push	edx\n\t"
		"mov	ecx,[ebx+0Ch]\n\t"
		"push	ecx\n\t"
		"call	_mcmd_engine_detour\n\t"
		"pop	ecx\n\t"
		"pop	edx\n\t"
		"mov	edx,engine\n\t"
		"test	eax,eax\n\t"
		"je		NOPROC%=\n\t"
		"mov	eax,[edx+08h]\n\t"
		"jmp	eax\n\t"
		"NOPROC%=:\n\t"
		"mov	eax,[edx+04]\n\t"
		"jmp	eax");*/