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
