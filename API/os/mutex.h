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

#ifndef __OS_MUTEX_H__
#define __OS_MUTEX_H__

/* Public forward declarations -----------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "common/object.h"

/* Public types --------------------------------------------------------------*/
typedef struct {
	object_t base;
} mutex_t;

/* Public macros -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public prototypes ---------------------------------------------------------*/
mutex_t *		mutex_new			(bool took);
bool			mutex_lock			(mutex_t *this,
							 int32_t ms);
void			mutex_unlock			(mutex_t *this);


#endif
