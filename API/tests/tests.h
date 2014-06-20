/*
	Copyright 2014 Chauveau Wilfried

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef __COMMON_TESTS_H__
#define __COMMON_TESTS_H__

/* Includes ------------------------------------------------------------------*/
#include <setjmp.h>
#include <stdint.h>

#define VERIFY_DIE_START \
	if (setjmp(g_on_die)==0) {
#define VERIFY_DIE_END \
	}

/* Public functions ----------------------------------------------------------*/
void				die_Expect			(char *expected_cause);
void				die_Verify			(void);

/* Public variables ----------------------------------------------------------*/
extern jmp_buf g_on_die;

#endif
