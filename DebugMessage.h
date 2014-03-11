/*-----------------------------------------------------------------------------
 * DebugMessage.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 06.06.2010 created
**---------------------------------------------------------------------------*/

#ifndef _debug_message
#define _debug_message

#if defined(_slogger_included)

#error DebugMessage.h 와 slogger 를 동시에 사용할 수 없습니다. slogger 를 사용하세요.

#else

#pragma message("[WARNING] "__FILE__" 는 더이상 관리되지 않습니다. 가능하면 _slogger 모듈을 사용하세요.")
#define DBG_OPN     DebugMessage(__FUNCTION__, __LINE__,
#define DBG_END     );
#define	DBG_ERR		DebugMessage(__FUNCTION__, __LINE__, "[ERR ]", 
#define DBG_WARN	DebugMessage(__FUNCTION__, __LINE__, "[WARN]",
#define DBG_INFO	DebugMessage(__FUNCTION__, __LINE__, "[INFO]",

#endif//DBG_XXX

void 
DebugMessage(
    const char* Function,
    int Line,
    const char* LogPrefix,    
    const char* fmt, 
    ...
    );


#endif//_debug_message