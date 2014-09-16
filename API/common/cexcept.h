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

#ifndef __COMMON_CEXCEPT_H__
#define __COMMON_CEXCEPT_H__

/* Public forward declarations -----------------------------------------------*/
typedef struct cexcept		cexcept_t;
typedef struct cexcept_ctx	cexcept_ctx_t;

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "cexcept_conf.h"

/* Public types --------------------------------------------------------------*/
/**
 * Open a try block.
 */
#define		Try \
	{ \
		void *__buf = cexcept_enter_ctx(); \
		switch (cexcept_jump(__buf)) { \
		case 0: \
		{ \
			do
/**
 * Catch variable declaration
 * @param	decl	Variable declaration to be used in Catch clause.
 */
#define		Catch \
			while(0); \
			break; \
		} \
		case 1: \
		{ \
			cexcept_t *e = cexcept_catch(); \
			(void)e; \
			do
/**
 * The finally clause. Will be run even if an exception is thrown.
 */
#define		Finally \
			while(0); \
			break; \
		} \
		default: \
			break; \
		} \
		cexcept_finally(); \
		switch(cexcept_jump(__buf)) { \
		case 0: \
		{ \
			do
/**
 * Close the try block and re-throw the exception if it was not caught.
 */
#define		EndTry \
			while(0); \
			break; \
		} \
		default: \
			break; \
		} \
		cexcept_exit_ctx(); \
	}

#define		Throw(type, message, is_dynamic) \
		cexcept_throw(type, message, is_dynamic);

/* Public macros -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public prototypes ---------------------------------------------------------*/
/**
 * Throws an exception.
 * @param	type		Exception type.
 * @param	message		Exception message.
 * @param	is_dynamic	true if message should be clean.
 */
void			cexcept_throw			(const char *type,
							 char *message,
							 bool is_dynamic);

/**
 * Saves previous exception context, prepare a new one.
 * saves current register's context and return true.
 * @return true on call, false when an exception occurs.
 */
void *			cexcept_enter_ctx		(void);
/**
 * Restores previous exception context.
 */
void			cexcept_exit_ctx		(void);

/**
 * Enter catch clause.
 * @return Thrown exception reference.
 */
cexcept_t *		cexcept_catch			(void);

/**
 * Enter finally clause.
 */
void			cexcept_finally			(void);

/**
 * @param	exception	Exception reference.
 * @return	Exception type string.
 */
const char *		cexcept_type			(cexcept_t *exception);
/**
 * @param	exception	Exception reference.
 * @return	Exception message string.
 */
const char *		cexcept_message			(cexcept_t *exception);

#endif

