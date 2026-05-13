/*
 * Copyright (c) 2022-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <cstdio>
#include <cstdlib>
#include "macro.h"

/* control the macro definition of DEBUG_LOG/_ERROR, DEBUG_CONDITIONAL_LOG/_ERROR,
 * before DEBUG_LOG/_ERROR print, all graph attributes wound be turned off */
#ifdef NDEBUG
#define UTIL_DEBUG_OPTION   OPTION_OFF // OPTION_ON, OPTION_OFF
#else
#define UTIL_DEBUG_OPTION   OPTION_ON  // OPTION_ON, OPTION_OFF
#endif

// some marco for DEBUG variable declaring and DEBUG operation
#if UTIL_DEBUG_OPTION
#define DEBUG_DECLARE(type, name_and_init) type name_and_init
#define DEBUG_STATEMENT(statement) do{ statement; } while(0)
#else
#define DEBUG_DECLARE(type, name_and_init) char identifier_cat(__UTIL_DEBUNG_VAR_, __COUNTER__)[0]
#define DEBUG_STATEMENT(statement) do{} while(0)
#endif


#define NONE_COLOR   GRAPH_ATTR_NONE
#define DEBUG_LOG_COLOR    GRAPH_FONT_YELLOW
#define DEBUG_ERROR_COLOR  GRAPH_FONT_RED

/* the default FILE pointer of LOG/ERROR print */
#define LOG_FILE   stdout
#define ERROR_FILE stdout

#define LOG_PRINT(format, ...)   fprintf(LOG_FILE,format, ##__VA_ARGS__)
#define ERROR_PRINT(format, ...) fprintf(ERROR_FILE,format, ##__VA_ARGS__)

#if UTIL_DEBUG_OPTION

#define DEBUG_LOG(format, ...)            \
do {                                      \
LOG_PRINT(NONE_COLOR DEBUG_LOG_COLOR);    \
LOG_PRINT("[LOG]: %s, ", __FILE__);       \
LOG_PRINT("func:%s, ", __FUNCTION__);     \
LOG_PRINT(format, ##__VA_ARGS__);         \
LOG_PRINT(NONE_COLOR"\n");                \
fflush(LOG_FILE);                         \
} while(0)

#define DEBUG_COND_LOG(cond, format, ...)   \
do {                                        \
if(cond) DEBUG_LOG(format, ##__VA_ARGS__);  \
} while(0)

#else //UTIL_DEBUG_OPTION

#define DEBUG_LOG(format, ...) do{} while(0)

#define DEBUG_COND_LOG(cond, format, ...) do{} while(0)

#endif //UTIL_DEBUG_OPTION


#if UTIL_DEBUG_OPTION

#define DEBUG_ERROR(format, ...)              \
do {                                          \
ERROR_PRINT(NONE_COLOR DEBUG_ERROR_COLOR);    \
ERROR_PRINT("[ERROR]: %s, ", __FILE__);       \
ERROR_PRINT("line:%i, ", __LINE__);           \
ERROR_PRINT("func:%s, ", __FUNCTION__);       \
ERROR_PRINT(format,  ##__VA_ARGS__);          \
ERROR_PRINT(NONE_COLOR"\n");                  \
fflush(ERROR_FILE);                           \
abort();                                      \
} while(0)

#define DEBUG_COND_ERROR(cond, format, ...)   \
do {                                          \
if(cond) DEBUG_ERROR(format, ##__VA_ARGS__);  \
} while(0)

#else //UTIL_DEBUG_OPTION

#define DEBUG_ERROR(format, ...) do{} while(0)

#define DEBUG_COND_ERROR(cond, format, ...) do{} while(0)

#endif //UTIL_DEBUG_OPTION

#endif //UTIL_DEBUG_H
