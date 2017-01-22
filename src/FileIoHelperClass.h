/**
 * @file    FileIoHelperClass
 * @brief   
 *
 * This file contains test code for `FileIoHelper` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011.10.13 created.
 * @copyright All rights reserved by Yonghwan, Noh.
**/
#pragma once

#include <string>
#include "StatusCode.h"


typedef class FileIoHelper
{
private:
	BOOL			mReadOnly;
	HANDLE			mFileHandle;
	LARGE_INTEGER   mFileSize;
	HANDLE			mFileMap;
	PUCHAR			mFileView;
public:
	FileIoHelper();
	~FileIoHelper();

	BOOL		Initialized()	{ return (INVALID_HANDLE_VALUE != mFileHandle) ? TRUE : FALSE;}
	BOOL		IsReadOnly()	{ return (TRUE == mReadOnly) ? TRUE : FALSE;}
	BOOL		IsLargeFile()	{ return ( mFileSize.QuadPart > 0 ? TRUE : FALSE ); }

	DTSTATUS	FIOpenForRead(IN std::wstring FilePath);
	DTSTATUS	FIOCreateFile(IN std::wstring FilePath, IN LARGE_INTEGER FileSize);
	void		FIOClose();

	DTSTATUS	FIOReference(
						IN BOOL ReadOnly, 
						IN LARGE_INTEGER Offset, 
						IN DWORD Size, 
						IN OUT PUCHAR& ReferencedPointer
						);
	void		FIOUnreference(
						);

	DTSTATUS	FIOReadFromFile(
						IN LARGE_INTEGER Offset, 
						IN DWORD Size, 
						IN OUT PUCHAR Buffer
						);

	DTSTATUS	FIOWriteToFile(
						IN LARGE_INTEGER Offset, 
						IN DWORD Size, 
						IN PUCHAR Buffer
						);

	const 
	PLARGE_INTEGER  FileSize(){ return &mFileSize; }
	

}*PFileIoHelper;