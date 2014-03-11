/**************************************************************************************
*
*	Copyright (C) 2007 AhnLab, Inc. All rights reserved.
*
*	This program is strictly confidential and not be used in outside the office...and 
*	some other copywrite agreement etc.
*
*	File:
*		D:\SVN.AHNLAB.CO.KR\Engine\Project\Ringer\1.0\Trunk\Src\downloader\Common\Singleton.h
*
*	Author:
*		Yonghwan.Noh ( somma@ahnlab.com )
*
*	DESCRIPTION :
*       reference count 기능을 가진 싱글톤 패턴 템플릿 클래스
*
*	HISTORY :
*  	2012-6-18 by Yonghwan.Noh
* 		1. file created
/**************************************************************************************/
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
        _ASSERTE(0 != mRefCount); if (0 == mRefCount) return;

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

#ifdef ALTONG_TEST_EXPORTS
    // debug / test 용 메소드
    static MyRefCount GetRefCount(){return mRefCount;}
    static T* GetInstanceForTest(){return mInstance;}
#endif//ALTONG_TEST_EXPORTS
};

template <typename T> T* Singleton<T>::mInstance = NULL;
template <typename T> MyRefCount Singleton<T>::mRefCount = 0;