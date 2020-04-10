/*-----------------------------------------------------------------------------
 * AKSyncObjs.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh Yong Hwan (fixbrain@gmail.com, unsorted@msn.com)
**---------------------------------------------------------------------------*/

#ifndef _SYNC_OBJS_H_
#define _SYNC_OBJS_H_

class CAbsSyncObjs
{
public:
    CAbsSyncObjs(){};
	virtual ~CAbsSyncObjs(){};

	virtual BOOL	Initialized(void) = 0;

	virtual BOOL	Init(void) = 0;
	virtual void	Terminate(void) = 0;

	virtual BOOL	Enter(void) = 0;
	virtual BOOL	Leave(void) = 0;

protected:
};



/**	-----------------------------------------------------------------------
	CriticalSection Wrapper 클래스
-------------------------------------------------------------------------*/
typedef
class AKCriticalSection: public CAbsSyncObjs
{
public:
	virtual BOOL	Enter(void);
	virtual BOOL	Leave(void);
	virtual BOOL    Initialized(void) { return m_initialized; }
protected:
private:
	BOOL				m_initialized;
	CRITICAL_SECTION	m_CriticalSection;	
public:
	AKCriticalSection(void): CAbsSyncObjs(), m_initialized(FALSE){}
	~AKCriticalSection(void)
	{
		// Terminate() 메소드는 virtual 이므로 호출하지 않고, 
		// 직접 구현 (걍 Terminate() 메소드를 없애버릴까?)
		// 
		if (TRUE == m_initialized)
		{
			DeleteCriticalSection(&m_CriticalSection);
			m_initialized = FALSE;
		}
	}

	/**	-----------------------------------------------------------------------
		\brief	initialize procedure
	-------------------------------------------------------------------------*/
	virtual BOOL	Init(void);
	
	/**	-----------------------------------------------------------------------
		\brief	terminate procedure
	-------------------------------------------------------------------------*/
	virtual void Terminate(void);
} *PAKCriticalSection;


class LockHelper
{
public:
    LockHelper(IN CAbsSyncObjs& Lock):m_Lock(Lock){}
    ~LockHelper(){m_Lock.Leave();}
private:
    CAbsSyncObjs& m_Lock;

    LockHelper(const LockHelper& rhs);
    LockHelper& operator= (const LockHelper& rhs);
};


#endif