/**----------------------------------------------------------------------------
 * ResourceHelper.h
 *-----------------------------------------------------------------------------
 * 자원 자동 해제를 위한 모듈 정의
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 22:9:2011   11:17 created
**---------------------------------------------------------------------------*/
#pragma once

#include "Win32Utils.h"

// 2012.12.20
// 더이상 사용하지 말고, shared_ptr 같은거 사용하자...하나씩 하나씩 없애자
//

// 복사 방지용 베이스 클래스
//  - member 가 없어서 가상 소멸자를 만들 필요가 없다.
//
class UnCopyable
{
protected:
	UnCopyable(){};
	~UnCopyable(){};
private:
	UnCopyable(const UnCopyable& rhs);
	UnCopyable& operator= (const UnCopyable& rhs);
};

// handle wrapper
//
class SmrtHandle: public UnCopyable
{
public:
	SmrtHandle(HANDLE handle) : m_handle(handle) {}
	~SmrtHandle(){ CloseHandle(m_handle); m_handle = INVALID_HANDLE_VALUE; }
private:
	HANDLE m_handle;
};

class SmrtObject: public UnCopyable
{
public:
	SmrtObject(PVOID object) : m_object(object) {}
	~SmrtObject(){ if (NULL != m_object) delete m_object; m_object = NULL;}
protected:
private:
	PVOID m_object;
};

// simple smart pointer
//
template <typename T>
class SmrtPtr: public UnCopyable
{
public:
	SmrtPtr(T& Ptr): m_ptr(Ptr) {};
	~SmrtPtr(){ free_and_nil(m_ptr);};
private:
	T& m_ptr;
};

class SmrtView: public UnCopyable
{
public:
	SmrtView(LPVOID hView) : m_view(hView) {}
	~SmrtView(){ UnmapViewOfFile(m_view); m_view = NULL;}
private:
	LPVOID m_view;
};
//
//class SmrtSvchandle: public UnCopyable
//{
//public:
//	SmrtSvchandle(SC_HANDLE h) : m_h(h) {}
//	~SmrtSvchandle(){ CloseServiceHandle(m_h); m_h=NULL;}
//private:
//	SC_HANDLE m_h;
//};
