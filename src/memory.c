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
 
#include "memory.h"

static mcmd_inline int _mcmd_memory_compare(mcmd_byte *data, 
					const mcmd_byte *pattern,
					const char *mask)
{
	for (; *mask; ++mask, ++data, ++pattern)
		if(*mask == 'x' && *data != *pattern)
			return 0;

	return (*mask) == 0;
}

mcmd_dword mcmd_memory_scan(char *pattern, char *mask)
{
	mcmd_dword i;
	mcmd_dword size;
	mcmd_dword address;
#ifdef SYS_WIN32
	MODULEINFO	info = { 0 };

	address = (mcmd_dword)GetModuleHandle(NULL);
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL),
			&info, sizeof(MODULEINFO));
	size = (mcmd_dword)info.SizeOfImage;
#else
	address = 0x804b4b0;
	size = 0x8147823 - address;
#endif
	for (i = 0; i < size; ++i)
		if (_mcmd_memory_compare((mcmd_byte *)(address + i),
					(mcmd_byte *)pattern, mask))
			return (mcmd_dword)(address + i);

	return 0;
}