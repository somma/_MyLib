/*-----------------------------------------------------------------------------
 * AKSyncObjs.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh Yong Hwan (fixbrain@gmail.com, unsorted@msn.com)
**---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "AKSyncObjs.h"

#include <crtdbg.h>



/**	-----------------------------------------------------------------------
	CriticalSection Wrapper 클래스
-------------------------------------------------------------------------*/

/**	-----------------------------------------------------------------------
	\brief	임계영역 진입
-------------------------------------------------------------------------*/
BOOL	AKCriticalSection::Enter(void)
{
	_ASSERTE(TRUE == m_initialized);
	if (FALSE == m_initialized)
	{
		return FALSE;
	}

    __try
    {
        ::EnterCriticalSection(&m_CriticalSection); 
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return FALSE;
    }        

	return TRUE;
}

/**	-----------------------------------------------------------------------
	\brief	임계영역 탈출
-------------------------------------------------------------------------*/
BOOL	AKCriticalSection::Leave(void)
{
	if (TRUE == m_initialized)
    {
        ::LeaveCriticalSection(&m_CriticalSection);
    }
    
    return TRUE;
}	

/**	-----------------------------------------------------------------------
	\brief	initialize procedure
-------------------------------------------------------------------------*/
BOOL	AKCriticalSection::Init(void)
{
	if (TRUE == m_initialized)
	{
		return TRUE;
	}

	__try
	{
		InitializeCriticalSection(&m_CriticalSection);
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return FALSE;
	}

	m_initialized = TRUE;
	return TRUE;		// for success
}


/**	-----------------------------------------------------------------------
	\brief	terminate procedure
-------------------------------------------------------------------------*/
void AKCriticalSection::Terminate(void)
{
	if (TRUE == m_initialized)
	{
		DeleteCriticalSection(&m_CriticalSection);
		m_initialized = FALSE;
	}
}



