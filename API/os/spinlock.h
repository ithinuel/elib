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

#ifndef __OS_SPINLOCK_H__
#define __OS_SPINLOCK_H__

/* Public forward declarations -----------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Public types --------------------------------------------------------------*/
typedef struct {
	volatile bool	is_locked;
} spinlock_t;

/* Public macros -------------------------------------------------------------*/
#define	spinlock_declare(var_name)		spinlock_t var_name = { .is_locked = false }

/* Public variables ----------------------------------------------------------*/
/* Public prototypes ---------------------------------------------------------*/
bool			spinlock_lock			(spinlock_t *this,
							 int32_t max_delay,
							 int32_t spin_delay);
void			spinlock_unlock			(spinlock_t *this);


#endif
