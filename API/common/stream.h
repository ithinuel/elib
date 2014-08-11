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

#ifndef __COMMON_STREAM_H__
#define __COMMON_STREAM_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "common/object.h"

/* Types ---------------------------------------------------------------------*/
typedef struct _stream		stream_t;

typedef void		(*stream_on_receive_f)		(void *ctx);
typedef int32_t		(*stream_read_f)		(stream_t *this,
				 	 	 	 uint8_t *buffer,
				 	 	 	 uint32_t len);
typedef int32_t		(*stream_write_f)		(stream_t *this,
				 	 	 	 uint8_t *buffer,
				 	 	 	 uint32_t len);

typedef struct
{
	stream_read_f		read;
	stream_write_f		write;
}	stream_ops_t;

struct _stream
{
	object_t		base;
	const stream_ops_t	*ops;
	stream_on_receive_f	on_receive;
};

/* Prototypes ----------------------------------------------------------------*/
/**
 * Read at most len data from this to buffer.
 * This call must not wait for data if none are available.
 *
 * @param	this	Stream.
 * @param	buffer	Buffer.
 * @param	len	Maximum data to read.
 * @return Byte quantity read or < 0 on error.
 */
int32_t		stream_read		(stream_t *this,
					 uint8_t *buffer,
					 uint32_t len);
/**
 * Write len data from buffer to this.
 * This call must block until all data are sent or an error occur.
 *
 * @param	this	Stream.
 * @param	buffer	Buffer.
 * @param	len	Byte count to write.
 * @return Byte count written to this stream.
 */
int32_t		stream_write		(stream_t *this,
					 uint8_t *buffer,
					 uint32_t len);

#endif
