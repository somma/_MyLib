/**
 * @file    send_ping.h
 * @brief   
 *
 * @author  Yonhgwhan, Noh (fixbrain@gmail.com)
 * @date    2016.05.30 10:47 created.
 * @copyright All rights reserved by Yonghwan, Noh.
**/

#pragma once
#include "stdafx.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <iphlpapi.h>
#include <Icmpapi.h>


typedef HANDLE (WINAPI *pIcmpCreateFile)();

typedef DWORD (WINAPI *pIcmpSendEcho)(
    _In_ HANDLE IcmpHandle,
    _In_ IPAddr DestinationAddress,
    _In_reads_bytes_(RequestSize) LPVOID RequestData,
    _In_ WORD RequestSize,
    _In_opt_ PIP_OPTION_INFORMATION RequestOptions,
    _Out_writes_bytes_(ReplySize) LPVOID ReplyBuffer,
    _In_range_(>= , sizeof(ICMP_ECHO_REPLY) + RequestSize + 8)
    DWORD ReplySize,
    _In_ DWORD Timeout
    );

typedef BOOL (WINAPI *pIcmpCloseHandle)(
    _In_ HANDLE  IcmpHandle
    );




/// @brief  send ping to target
/// @return return true if pong received else return fasle
bool send_ping(_In_ const wchar_t* target_ip_str)
{
    _ASSERTE(NULL != target_ip_str);
    if (NULL == target_ip_str) return false;


    // check IcmpCreateFile() is presense.
    static HMODULE hmod = NULL;
    static pIcmpCreateFile _IcmpCreateFile = NULL;
    static pIcmpSendEcho _IcmpSendEcho = NULL;
    static pIcmpCloseHandle _IcmpCloseHandle = NULL;
    

    if (NULL == hmod)
    {
        hmod = GetModuleHandleW(L"Iphlpapi.dll");
        if (NULL == hmod)
        {
            hmod = LoadLibraryW(L"Iphlpapi.dll");
            if (NULL == hmod)
            {
                log_err "no Iphlpapi.dll. give up. gle=%u", GetLastError() log_end;
                return false;
            }
        }
    }

    _ASSERTE(NULL != hmod);
     
    if (NULL==_IcmpCreateFile)
        _IcmpCreateFile = (pIcmpCreateFile)GetProcAddress(hmod, "IcmpCreateFile");
    
    if (NULL == _IcmpSendEcho)
        _IcmpSendEcho = (pIcmpSendEcho)GetProcAddress(hmod, "IcmpSendEcho");

    if (NULL == _IcmpCloseHandle)
        _IcmpCloseHandle = (pIcmpCloseHandle)GetProcAddress(hmod, "IcmpCloseHandle");

    if (NULL == _IcmpCreateFile || NULL == _IcmpSendEcho || NULL == _IcmpCloseHandle)
    {
        log_err "GetProcAddress() failed. " log_end;
        return false;
    }


    in_addr target_addr = { 0 };
    if (!str_to_ipv4(target_ip_str, target_addr))
    {
        log_err "str_to_ipv4( %s ) failed.", target_ip_str log_end;
        return false;
    }

    HANDLE h_icmp = INVALID_HANDLE_VALUE;
    char  ping_buf[16] = "ping";
    uint32_t pong_buf_len = sizeof(ICMP_ECHO_REPLY) + sizeof(ping_buf);
    char* pong_buf = (char*)malloc(pong_buf_len);
    if (NULL == pong_buf)
    {
        log_err "malloc() failed." log_end;
        return false;
    }


    bool pong = false;
    do
    {
        h_icmp = _IcmpCreateFile();
        if (INVALID_HANDLE_VALUE == h_icmp)
        {
            log_err "IcmpCreateFile() failed. gle=%u", GetLastError() log_end;
            break;
        }
    
        DWORD ret = _IcmpSendEcho(
                        h_icmp, 
                        target_addr.S_un.S_addr, 
                        ping_buf, 
                        sizeof(ping_buf), 
                        NULL, 
                        pong_buf, 
                        pong_buf_len, 
                        1000);
        if (0 != ret)
        {
            PICMP_ECHO_REPLY echo_reply = (PICMP_ECHO_REPLY)pong_buf;
            in_addr reply_addr = { 0 };
            reply_addr.S_un.S_addr = echo_reply->Address;
        
            log_dbg "ping -> %ws ->", target_ip_str log_end;
            log_dbg "     <- %ws pong", ipv4_to_str(reply_addr).c_str() log_end;

            pong = true;
        }
        else
        {
            log_dbg "ping -> %ws ->", target_ip_str log_end;
            log_dbg "     <- none " log_end;

            pong = false;
        }
    
    } while (false);

    free_and_nil(pong_buf);
    _IcmpCloseHandle(h_icmp);
    return pong;
}
