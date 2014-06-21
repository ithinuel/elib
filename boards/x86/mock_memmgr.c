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

#include <stdlib.h>
#include <string.h>
#include "unity_fixture.h"
#include "os/memmgr.h"

/* Those declaration would be better in unity_fixture_malloc_overrides or unity_fixture */
void * unity_malloc(size_t);
void * unity_calloc(size_t, size_t);
void * unity_realloc(void *, size_t);
void unity_free(void*);

static void *mm_zalloc_mock(uint32_t size)
{
	void *ptr = malloc(size);
	if (ptr != NULL) {
		memset(ptr, 0, size);
	}
	return ptr;
}

static void mm_free_mock(void *ptr)
{
	free(ptr);
}

void mock_memmgr_setup(void)
{
	UT_PTR_SET(mm_zalloc, mm_zalloc_mock);
	UT_PTR_SET(mm_free, mm_free_mock);
}
