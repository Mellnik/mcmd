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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "engine.h"
#include "memory.h"
#include "main.h"

static struct engine_addr_data *engine_addr;

struct engine_pawn_rel *engine_pawn;

static void engine_env_init(void)
{
	int i;

	engine_addr = (struct engine_addr_data *)
			malloc(sizeof(struct engine_addr_data));
	assert(engine_addr != NULL);

	engine_pawn = (struct engine_pawn_rel *)
			malloc(sizeof(struct engine_pawn_rel));
	assert(engine_pawn != NULL);

	engine_pawn->gamemode = NULL;
	engine_pawn->gamemode_idx = 0;

	for (i = 0; i < PAWN_MAX_AMX; ++i)
		engine_pawn->scripts[i] = NULL;

	for (i = 0; i < PAWN_MAX_PLAYERS; ++i)
		engine_pawn->reqtime[i] = INVALID_REQ_TIME;
}

static void engine_grab_addresses(mcmd_dword *start, mcmd_dword *fail, mcmd_dword *success)
{
#ifdef SYS_WIN32
	/* Address of where we hook in */
	*start = mcmd_memory_scan("\x8B\x04\xB7\x85\xC0\x74\x63", 
					"xxxxxxx");
	/* Address of fail jump (if we want "SERVER: Unknown Command") */
	*fail = mcmd_memory_scan("\x83\xFE\x10\x7C\x90\x8B\x44\x24\x10\x5F\x5E",
					"xxxxxxxxxxx") + 0x5;
	/* Address of where we going to jump upon mcmd done */
	*success = mcmd_memory_scan("\x83\xC4\x08\xC2\x08\x00\x5F", 
					"xxxxxxx") + 0x6;
#else /* 0x80A0A80 function start, 0.3z-R2-2 */
	/* 0x80A0A93 */
	*start = mcmd_memory_scan("\x8B\x7D\x00\x89\xF3\xEB\x00\x8D\xB6\x00\x00\x00\x00\x83\xC3\x00\x8D\x46\x00\x39\xC3\x0F\x8F\x00\x00\x00\x00\x8B\x13\x85\xD2\x74\x00\x89\x14\x24\x8D\x45\x00\x89\x44\x24\x00\xB8\x00\x00\x00\x00\x89\x44\x24\x00\xE8\x00\x00\x00\x00\x85\xC0\x75\x00\x89\x7C\x24\x00\x31\xC0\x31\xC9\x89\x44\x24\x00", 
					"xx?xxx?xxxxxxxx?xx?xxxx????xxxxx?xxxxx?xxx?x????xxx?x????xxx?xxx?xxxxxxx?");
	/* 0x80A0B44 */
	*fail = mcmd_memory_scan("\x85\xD2\x0F\x84\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xEB\x00\x8B\x45\x00", 
					"xxxx????x????x?xx?") + 0xF;
	/* 0x80A0B3D */
	*success = *fail - 0x7;
#endif
}

static int engine_command_detour(int playerid, char *cmdtext)
{
	char	cmd[PAWN_MAX_FUNC_SIZE] = PAWN_CMD_PREFIX;
	AMX		*cmd_amx = NULL;
	int		cmd_idx = 0;
	int		i;
	int		pos = 0;
	cell	amx_rel = 0;
	cell	amx_ret = 0;

	while (cmdtext[++pos] != ' ' && pos < PAWN_MAX_FUNC_SIZE - 3)
		cmd[pos + 2] = tolower(cmdtext[pos]);

	if (cmdtext[pos++] != ' ') { /* loop above stopped because command name too long */
		/* go to position after space char */
		while (cmdtext[pos] != ' ' && cmdtext[pos] != '\0')
			pos++;

		if (cmdtext[pos] != '\0')
			pos++;
	}

	for (i = 0; i < PAWN_MAX_AMX && engine_pawn->scripts[i] != NULL; ++i)
		if(amx_FindPublic(engine_pawn->scripts[i], cmd, &cmd_idx) == AMX_ERR_NONE) {
			cmd_amx = engine_pawn->scripts[i];
			break;
		}

	engine_pawn->reqtime[playerid] = (int)time(NULL);

	if (engine_pawn->gamemode != NULL) {
		amx_Push(engine_pawn->gamemode, cmd_amx != NULL);
		amx_PushString(engine_pawn->gamemode, &amx_rel, NULL, cmdtext, 0, 0);
		amx_Push(engine_pawn->gamemode, playerid);

		amx_Exec(engine_pawn->gamemode, &amx_ret, engine_pawn->gamemode_idx);
		amx_Release(engine_pawn->gamemode, amx_rel);

		if (amx_ret && cmd_amx != NULL)
			goto exec_cmd;

		return 1;
	}

	/* no callback + command doesn't exist: return "SERVER: Unknown Command" */
	if (cmd_amx == NULL)
		return 0;

exec_cmd:
	/* Final command call */
	amx_PushString(cmd_amx, &amx_rel, NULL, cmdtext + pos, 0, 0);
	amx_Push(cmd_amx, playerid);

	amx_Exec(cmd_amx, &amx_ret, cmd_idx);
	amx_Release(cmd_amx, amx_rel);
	return 1;
}

#ifdef SYS_WIN32
_declspec(naked) void engine_opct_hook(void)
{
	_asm /* EBX = cmdtext, EBP = playerid */
	{
		push	ebx
		push	ebp
		call	engine_command_detour
		pop		ebp
		pop		ebx
		mov		edx, dword ptr [engine_addr]
		test	eax, eax
		je		noproc
		mov		eax, [edx+08h] ; engine_addr->success
		jmp		eax
	noproc:
		mov		eax, [edx+04h] ; engine_addr->fail
		jmp		eax
	}
}
#else
void engine_opct_hook(void)
{
	__asm__ volatile( /* EBP-14h = cmdtext, EBP+0Ch = playerid */
		".intel_syntax noprefix\n"
		"add esp, 8\n"
		"pop ebp\n"
		"mov edx, [ebp-0x14]\n"
		"push edx\n"
		"mov ecx, [ebp+0xC]\n"
		"push ecx\n"
		"call engine_command_detour\n"
		"pop ecx\n"
		"pop edx\n"
		"mov edx, [engine_addr+0x4]\n"
		"jmp [edx+0x3]\n"
		".att_syntax\n");

		/*
		"pop edx\n"
		"mov edx, [engine+0x4]\n"
		"add edx, 0x3\n"
		"jmp edx\n"
		"test eax, eax\n"
		"je NOPROC%=\n"
		"mov eax, [edx+0x8]\n"
		"jmp [eax+0x3]\n"
		"NOPROC%=:\n"
		"mov eax, [edx+0x4]\n"
		"jmp eax\n"*/
		//".att_syntax\n");
}
#endif

int engine_install(void)
{
	mcmd_dword protback, distance, len, i;

	engine_env_init();

	engine_grab_addresses(&engine_addr->start, &engine_addr->fail, &engine_addr->success);

	if(engine_addr->start <= 0 || engine_addr->fail <= 0 || engine_addr->success <= 0)
		return 0;

#ifdef SYS_WIN32
	len = engine_addr->fail - engine_addr->start;
#else
	/* Success code comes before fail mov on Linux */
	len = engine_addr->success - engine_addr->start;
#endif
	
	/* Calculate distance to our detour, -0x5 for jmp size */
	distance = ((mcmd_dword)engine_opct_hook - engine_addr->start) - 0x5;

	/* Make memory writeable */
#ifdef SYS_WIN32
	VirtualProtect((mcmd_byte *)engine_addr->start, len, PAGE_EXECUTE_READWRITE, &protback);
#else
	size_t 
		pagesize = getpagesize(),
		mem_addr = (((size_t)engine_addr->start / pagesize) * pagesize),
		mem_count = ((size_t)len / pagesize) * pagesize + pagesize * 2;
	mprotect((void *)mem_addr, (size_t)mem_count, PROT_EXEC | PROT_READ | PROT_WRITE);
#endif
	/* Write jmp + our address */
	*(mcmd_byte *)engine_addr->start = 0xE9;
	*(mcmd_dword *)(engine_addr->start + 0x1) = distance;

	/* nop code after our jmp to end address*/
	for(i = 0x5; i < len; ++i)
		*(mcmd_byte *)(engine_addr->start + i) = 0x90;

	/* Be nice and restore old permission */
#if defined __WIN32__ || defined _WIN32 || defined WIN32
	VirtualProtect((mcmd_byte *)engine_addr->start, len, protback, &protback);
#else
	mprotect((void *)mem_addr, (size_t)mem_count, PROT_READ | PROT_EXEC);
#endif
	return 1;
}

void engine_exit(void)
{
	free(engine_addr);
	free(engine_pawn);
}

void engine_dump_memory(void)
{
	if (engine_addr == NULL) 
		return;
		
	logprintf("[mcmd] Dump engine_addr->start: 0x%X", 
			engine_addr->start);
	logprintf("[mcmd] Dump engine_addr->fail: 0x%X", 
			engine_addr->fail);
	logprintf("[mcmd] Dump engine_addr->success: 0x%X", 
			engine_addr->success);
	logprintf("[mcmd] Dump engine_pawn->gamemode: 0x%X", 
			engine_pawn->gamemode);
	logprintf("[mcmd] Dump engine_pawn->gamemode_idx: %i", 
			engine_pawn->gamemode_idx);
}