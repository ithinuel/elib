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
#include <setjmp.h>
#include <stdlib.h>
#include <stddef.h>
#include "cexcept/cexcept.h"
#include "common/common.h"
#include "common/cexcept.h"
#include "os/memmgr.h"
#include "os/task.h"

/* Types definitions ---------------------------------------------------------*/
struct cexcept_ctx
{
	cexcept_ctx_t	*prev;
	jmp_buf		jmpbuf;
	uint8_t		state;

	cexcept_t	excpt;
	bool		is_set;
	bool		is_caught;
};

/* Public functions ----------------------------------------------------------*/
void cexcept_throw(const char *type, char *message, bool is_dynamic)
{
	cexcept_ctx_t *cur = task_cexcept_get_ctx();

	if (cur == NULL) {
		die("Throw without context");
	}

	if (cur->is_set && cur->excpt.is_dynamic) {
		mm_free(cur->excpt.message);
	}

	cur->excpt.type = type;
	cur->excpt.message = message;
	cur->excpt.is_dynamic = is_dynamic;
	cur->is_set = true;
	cur->is_caught = false;
	longjmp(cur->jmpbuf, cur->state);
}

void *cexcept_enter_ctx(void)
{
	cexcept_ctx_t *new = mm_zalloc(sizeof(cexcept_ctx_t));
	if (new == NULL) {
		cexcept_throw("NOMEM", "no memory available to enter cexcept ctx.", false);
	}
	new->prev = task_cexcept_get_ctx();
	new->is_set = false;
	new->is_caught = false;
	new->state = 1;
	task_cexcept_set_ctx(new);

	return new->jmpbuf;
}

cexcept_t *cexcept_catch(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	if (ctx == NULL) {
		die("Catch without context");
	}
	ctx->state = 2;
	ctx->is_caught = true;
	return &ctx->excpt;
}

void cexcept_finally(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	if (ctx == NULL) {
		die("Finally without context");
	}
}


void cexcept_exit_ctx(void)
{
	cexcept_ctx_t	ctx = {0};
	cexcept_ctx_t	*cur = task_cexcept_get_ctx();
	if (cur == NULL) {
		die("EndTry without context");
	}

	ctx = *cur;
	mm_free(cur);
	task_cexcept_set_ctx(ctx.prev);

	if (ctx.is_set && !ctx.is_caught) {
		cexcept_throw(ctx.excpt.type, ctx.excpt.message, ctx.excpt.is_dynamic);
	} else if (ctx.excpt.is_dynamic && (ctx.excpt.message != NULL)) {
		mm_free(ctx.excpt.message);
	}
}
