/**
 * @file    WOW64 util
 * @brief
 *
 * @author  Yonghwan, Roh (somma@somma.kr)
 * @date    07.14.2022 21:59 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#define _Wow64Util_H_

#include <Windows.h>

bool is_wow64_process(HANDLE hProcess);
bool is_wow64_process(DWORD pid);
bool is_wow64_message();
bool disable_wow64_fs_redirection(_Out_ void**& revert_handle);
bool revert_wow64_fs_redirection(_In_ void* revert_handle);
