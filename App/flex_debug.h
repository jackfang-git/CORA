
#ifndef __FLEX_DEBUG_H__
#define __FLEX_DEBUG_H__

#include <stdio.h>
#include "SEGGER_RTT.h"

#ifdef FLEX_DEBUG
#define flex_debug(fmt, arg...)\
do {                           \
	SEGGER_RTT_printf(0,fmt, ##arg);	\
} while(false)

#define flex_error(fmt, arg...)\
do {                           \
	SEGGER_RTT_printf(0,"---- File: %s Line: %d ----\r\n ERROR: ",__FILE__, __LINE__); \
	SEGGER_RTT_printf(0,fmt, ##arg);         \
} while(false)
#else
#define flex_debug(fmt, arg...)\
do {                           \
} while(false)
#define flex_error(fmt, arg...)\
do {                           \
} while(false)

#endif

#endif
