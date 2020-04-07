/*-----------------------------------------------------------------------------
 * ThreadManager.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 2010.02.09 created
 * - 2013.08-08 modified
 *		StatusCode.h 제거
 *		FreeAndNil() 함수 사용 안함
**---------------------------------------------------------------------------*/

#ifndef _dt_thread_manager_
#define _dt_thread_manager_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define		CONTEXT_TERMINATED_EVENT_TIMEOUT		10 * 1000				// 10 sec

/* 
 * KillEvent 를 모니터링 하도록 프로시저를 작성하면 됨
 *

DWORD __stdcall ThreadProcedure(IN LPVOID lpThreadParam)
{
    _ASSERTE(NULL != lpThreadParam);
    if (NULL == lpThreadParam) return 0;

    
    PSOME_OBJECT so = (PSOME_OBJECT)lpThreadParam;
            or
    PDTTHREAD_CONTEXT pdc = (PDTTHREAD_CONTEXT) lpThreadParam;

    while (TRUE)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(so->pdc->KillEvent, 0))
                            or
        if (WAIT_OBJECT_0 == WaitForSingleObject(pdc->KillEvent, 0))
        {
            break;
        }

        ...... implementation .......
    }

    return 0;
}

*/

typedef struct _DTTHREAD_CONTEXT
{
    HANDLE      ThreadHandle;
    HANDLE      KillEvent;
    DWORD       ThreadId;

    LPVOID      ThreadParam;
    LPTHREAD_START_ROUTINE ThreadProcedure;
} DTTHREAD_CONTEXT, *PDTTHREAD_CONTEXT;


PDTTHREAD_CONTEXT 
CreateThreadContext(
    IN LPTHREAD_START_ROUTINE ThreadProcedure,
    IN LPVOID Threadparam
    );

void 
DestroyThreadContext(IN PDTTHREAD_CONTEXT& ctx);

#endif//_dt_thread_manager_