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

/* Public forward declarations -----------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdbool.h>
#include "common/stream.h"

/* Private prototypes --------------------------------------------------------*/
static bool		stream_is_valid			(stream_t *this);

/* Private functions ---------------------------------------------------------*/
static bool stream_is_valid(stream_t *this)
{
	return ((this != NULL) && (this->ops != NULL));
}

/* Public functions ----------------------------------------------------------*/
int32_t stream_read(stream_t *this, uint8_t *buffer, uint32_t len)
{
	if (stream_is_valid(this)) {
		if (this->ops->read != NULL) {
			return this->ops->read(this, buffer, len);
		}
	}
	return -1;
}

int32_t stream_write(stream_t *this, uint8_t *buffer, uint32_t len)
{
	if (stream_is_valid(this)) {
		if (this->ops->write != NULL) {
			return this->ops->write(this, buffer, len);
		}
	}
	return -1;
}

