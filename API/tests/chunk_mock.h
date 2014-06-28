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

#ifndef __TESTS_CHUNK_MOCK_H__
#define __TESTS_CHUNK_MOCK_H__

#include <stdint.h>
#include "memmgr/chunk.h"

void		mock_chunk_setup			(void);
void		mock_chunk_verify			(void);
void		mock_mm_find_first_free_ExpectAndReturn	(uint16_t wanted_csize,
							 mm_chunk_t *ret);
void		mock_mm_chunk_split_ExpectAndReturn	(mm_chunk_t *this,
							 uint16_t csize,
							 bool do_ret);
void		mock_mm_chunk_merge_Expect		(mm_chunk_t *this);

#endif
