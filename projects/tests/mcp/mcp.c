
#include <stdlib.h>
#include <stdbool.h>
#include "unity_fixture.h"

#include "mcp/mcp.h"

static void mcp_entry(void);

system_entry_t g_mcp_entry  =
{
	.entry = mcp_entry,
	.stack_size = 512,
	.priority = 1
};

static void runAllTests()
{
	RUN_TEST_GROUP(mm_chunk);
	RUN_TEST_GROUP(memmgr);
	RUN_TEST_GROUP(cexcept);
	RUN_TEST_GROUP(object);
	RUN_TEST_GROUP(mutex);
	RUN_TEST_GROUP(spinlock);
	RUN_TEST_GROUP(stream);
}

static void mcp_entry(void)
{
	UnityMain(0, NULL, runAllTests);
}
