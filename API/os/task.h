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

#ifndef __OS_TASK_H__
#define __OS_TASK_H__

/* Public forward declarations -----------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "common/cexcept.h"
#include "common/mockable.h"

/* Public types --------------------------------------------------------------*/
typedef void		(*task_delay_ms_f)		(int32_t ms);

/* Public macros -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public prototypes ---------------------------------------------------------*/

/**
 * Block current task for ms.
 * @param ms	period in ms.
 */
MOCKABLE task_delay_ms_f		task_delay_ms;

cexcept_ctx_t *		task_cexcept_get_ctx		(void);
void			task_cexcept_set_ctx		(cexcept_ctx_t *);

#endif
