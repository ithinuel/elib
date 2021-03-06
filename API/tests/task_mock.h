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

#ifndef __TESTS_TASK_MOCK_H__
#define __TESTS_TASK_MOCK_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Types ---------------------------------------------------------------------*/
typedef void (*task_delay_ms_delegate_f)(void);

/* Prototypes ----------------------------------------------------------------*/
void			task_mock_delay_ms_setup	(void);
void			task_mock_delay_ms_verify	(void);
void			task_delay_ms_ExpectNthenCbk	(uint32_t ms,
							 uint32_t times,
							 task_delay_ms_delegate_f cbk);

#endif
