/*-----------------------------------------------------------------------------
 * DebugMessage.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 06.06.2010 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DebugMessage.h"
#include <crtdbg.h>
#include <strsafe.h>

PCH get_filename_part(IN PCH full_path, IN DWORD length);


/**----------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
void 
DebugMessage(
    const char* Function,
    int Line,
    const char* LogPrefix,
    const char* fmt, 
    ...
    )
{
    _ASSERTE(NULL != LogPrefix); if(NULL==LogPrefix) return;
    _ASSERTE(NULL != fmt); if(NULL==fmt) return;

    char log_buffer[4096]={0};
    size_t remain = sizeof(log_buffer);
    char* pos = log_buffer;
	va_list args;	

    // 프로세스 정보
    // 
    char    name_buffer[MAX_PATH+1]={0};
    char    *name = name_buffer; 
    name_buffer[0] = '\0';
    DWORD len = GetModuleFileNameA(NULL, name_buffer, sizeof(name_buffer));
    if (len>0) name = get_filename_part(name_buffer, len);
	
	// format string 파라미터를 Multibyte / WideChar 가 잘 못 들어온 경우등
    // 에러가 발생할 수 있어서 해당 함수를 찾기위한 디버그 코드 
    // e.g. %s => L"debug string...."    or %S => "debug string"
    // 
    size_t remain_b = remain;
    char* pos_b = pos;

	HRESULT hRes = StringCbPrintfExA(
                        pos, 
                        remain, 
                        &pos,
                        &remain,
                        0, 
                        "%-18s(%04u:%04u), %s %s() ", 
                        name, 
                        GetCurrentProcessId(),
                        GetCurrentThreadId(),
                        LogPrefix, 
                        Function
                        );
    if (S_OK != hRes)
    {
	    OutputDebugStringA("StringCbPrintfExA() failed, more log buffer needed...?");
	    return;
    }
    remain_b = remain;
    pos_b = pos;


    va_start(args,fmt);
    hRes = StringCbVPrintfExA(
                        pos, 
                        remain, 
                        &pos,
                        &remain,
                        0, 
                        fmt, 
                        args
                        );

    if (S_OK != hRes)
    {
		// 한글 같은 unicode 문자열이 있는 경우 발생할 수 있음
		// 
        StringCbPrintfExA(
            pos_b, 
            remain_b, 
            &pos_b, 
            &remain_b, 
            0, 
            "invalid function call parameters, line=%d", 
            Line
            );		
        remain = remain_b;
        pos = pos_b;
    }    
    va_end(args);

    hRes = StringCbPrintfExA(
                        pos, 
                        remain, 
                        &pos,
                        &remain, 
                        0, 
                        "%s",
                        "\r\n"
                        );
    if (!SUCCEEDED(hRes))
    {
		
        OutputDebugStringA("StringCbPrintfExA() failed, more log buffer needed...?");
        return;
    }

    OutputDebugStringA(log_buffer);    
}


/**----------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
PCH
get_filename_part(
    IN  PCH full_path,
    IN  DWORD length
    )
{
    PCH p = NULL;

    if (length==0) return NULL;
    p = &full_path[length];
    
    while(p>=full_path)
    {
        if (*p=='\\') return p+1;
        --p;
    }

    return NULL;
}

