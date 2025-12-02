/**
 * @file    log_internal.h
 * @brief   Internal utilities for log module. Do NOT include this file directly.
 *          Use log.h for public API.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2024-12-02 created
 * @copyright (C)Somma, Inc. All rights reserved.
**/

#pragma once

//=============================================================================
// DPFLTR constants (from dpfilter.h)
//=============================================================================

#define LOG_INTERNAL_DPFLTR_ERROR_LEVEL     0
#define LOG_INTERNAL_DPFLTR_WARNING_LEVEL   1
#define LOG_INTERNAL_DPFLTR_TRACE_LEVEL     2
#define LOG_INTERNAL_DPFLTR_INFO_LEVEL      3
#define LOG_INTERNAL_DPFLTR_MASK            0x80000000
#define LOG_INTERNAL_DPFLTR_IHVDRIVER_ID    77

//=============================================================================
// Internal debug output functions
//=============================================================================

namespace log_internal
{
    /// @brief  Function pointer type for DbgPrintEx
    typedef ULONG(__cdecl* fnDbgPrintEx)(
        _In_ ULONG ComponentId,
        _In_ ULONG Level,
        _In_z_ _Printf_format_string_ PCSTR Format,
        ...
    );

    /// @brief  Get DbgPrintEx function pointer (lazy initialization)
    inline fnDbgPrintEx get_dbg_print_fn()
    {
        static fnDbgPrintEx _fn = nullptr;
        if (nullptr == _fn)
        {
            HMODULE nt = GetModuleHandleW(L"ntdll.dll");
            if (nt)
            {
                _fn = (fnDbgPrintEx)GetProcAddress(nt, "DbgPrintEx");
            }
        }
        return _fn;
    }

    /// @brief  Output raw message to debugger using DbgPrintEx
    /// @param  level   DPFLTR level (LOG_INTERNAL_DPFLTR_xxx)
    /// @param  msg     Message to output
    inline void dbg_output(_In_ ULONG level, _In_ const char* msg)
    {
        fnDbgPrintEx fn = get_dbg_print_fn();
        if (fn)
        {
            fn(LOG_INTERNAL_DPFLTR_IHVDRIVER_ID, level, "%s", msg);
        }
        else
        {
            OutputDebugStringA(msg);
        }
    }

    /// @brief  Output formatted debug log with PID:TID prefix
    /// @param  level       DPFLTR level
    /// @param  func_name   Function name (__FUNCTION__)
    /// @param  format      Printf-style format string
    inline void dbg_log(
        _In_ ULONG level,
        _In_ const char* func_name,
        _In_z_ _Printf_format_string_ const char* format,
        ...
    )
    {
        char msg_buf[512] = { 0 };
        va_list args;
        va_start(args, format);
        vsnprintf_s(msg_buf, sizeof(msg_buf), _TRUNCATE, format, args);
        va_end(args);

        char log_buf[1024];
        sprintf_s(log_buf, sizeof(log_buf),
            "[%s] (%5u:%5u) : %s : %s\n",
            (level == LOG_INTERNAL_DPFLTR_ERROR_LEVEL) ? "ERR " :
            (level == LOG_INTERNAL_DPFLTR_WARNING_LEVEL) ? "WARN" : "DEBG",
            GetCurrentProcessId(), GetCurrentThreadId(),
            func_name, msg_buf);
        dbg_output(level, log_buf);
    }

} // namespace log_internal
