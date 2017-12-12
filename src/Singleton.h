/**----------------------------------------------------------------------------
 * Singleton.h
 *-----------------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 26:8:2011   15:34 created
**---------------------------------------------------------------------------*/

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _M_AMD64

#define MyRefCount                      LONGLONG
#define MyInterlockedIncrement          InterlockedIncrement64
#define MyInterlockedDecrement          InterlockedDecrement64
#define MyInterlockedCompareExchange    InterlockedCompareExchange64
#define MyInterlockedCompareExchange    InterlockedCompareExchange64
#define MyInterlockedAnd                _InterlockedAnd64

#elif defined (_M_IX86)

#define MyRefCount                      LONG
#define MyInterlockedIncrement          InterlockedIncrement
#define MyInterlockedDecrement          InterlockedDecrement
#define MyInterlockedCompareExchange    InterlockedCompareExchange
#define MyInterlockedCompareExchange    InterlockedCompareExchange
#define MyInterlockedAnd                _InterlockedAnd

#else
#error !!! Need to write code for this architecture?
#endif


template <typename T>
class Singleton
{
private:
    static MyRefCount   mRefCount;
    static T*           mInstance;

    Singleton(){ }
    ~Singleton(){}

    static void IncrementReferenceCount()
    {
        _ASSERTE(NULL != mInstance);
        MyInterlockedIncrement(&mRefCount);
    }

    static void DecrementReferenceCount()
    {
        if (0 == mRefCount) return;

		_ASSERTE(nullptr != mInstance);
        MyInterlockedDecrement(&mRefCount);
        if (0 == MyInterlockedCompareExchange(&mRefCount, 0, 0))
        {
            delete mInstance; mInstance = NULL;
        }
    }
public:
    static T* GetInstancePointer()
    {
        if (NULL == mInstance) {mInstance = new T();}
        Singleton::IncrementReferenceCount();
        return mInstance;
    }
	
	static void ReleaseInstance()
    {
        Singleton::DecrementReferenceCount();
    }
};

template <typename T> T* Singleton<T>::mInstance = nullptr;
template <typename T> MyRefCount Singleton<T>::mRefCount = 0;