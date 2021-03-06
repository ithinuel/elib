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


/* Includes ------------------------------------------------------------------*/
#include "common/common.h"

/* Public functions ----------------------------------------------------------*/
uint32_t umin(uint32_t a, uint32_t b)
{
	return (a<b)? a : b;
}

char *bool_to_string(bool val)
{
	return val?"true":"false";
}
