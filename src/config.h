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

#ifndef _CONFIG_H_
#define _CONFIG_H_
 
#if defined __WIN32__ || defined _WIN32 || defined WIN32

    #define WIN32_LEAN_AND_MEAN
	#define mcmd_inline __inline

#else

	#define mcmd_inline inline
	#ifndef NULL
		#define NULL 0
	#endif
	#include <dlfcn.h>
	
#endif

#define PAWN_MAX_FUNC_SIZE 32
#define PAWN_MAX_AMX 17
#define PAWN_MAX_PLAYERS 1000
#define PAWN_CMD_PREFIX "_ce"

typedef unsigned long mcmd_dword;
typedef unsigned char mcmd_byte;

// Useful links:
// https://sourceware.org/gdb/onlinedocs/gdb/Machine-Code.html
// https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
// http://stackoverflow.com/questions/16354348/how-to-pass-variables-to-intel-format-inline-asm-code-compiled-with-gcc

#endif