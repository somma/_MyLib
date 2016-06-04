/**
 * @file    
 * @brief   
 * 
 * This file contains 
 *
 * @author  Yonhgwhan, Noh (fixbrain@gmail.com)
 * @date    2016:06:04 10:02 created.
 * @copyright All rights reserved by Yonghwan, Noh.
**/
#include "stdafx.h"
#include "wmi_client.h"


WmiClient::WmiClient():_initialized(false), _locator(NULL), _svc(NULL)
{
}


WmiClient::~WmiClient()
{
    finalize();
}


bool WmiClient::initialize()
{
    // #1, init COM
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (!SUCCEEDED(hres))
    {
        log_err "CoInitializeEx() failed. hres=%u", hres log_end;
        return false;
    }

    // #2, set general COM security levels
    hres = CoInitializeSecurity(NULL,
                                -1,                             // COM authentication
                                NULL,                           // Authentication services
                                NULL,                           // Reserved
                                RPC_C_AUTHN_LEVEL_DEFAULT,      // Default authentication
                                RPC_C_IMP_LEVEL_IMPERSONATE,    // Default Impoersonation
                                NULL,                           // Authentication info
                                EOAC_NONE,                      // Additional capabilities
                                NULL                            // Reserved
                                );
    if (!SUCCEEDED(hres))
    {
        log_err "CoInitializeSeciruty() failed. hres=%u", hres log_end;
        CoUninitialize();
        return false;
    }

    // #3, Obtain the initial locator to WMI
    hres = CoCreateInstance(CLSID_WbemLocator,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (LPVOID*)&_locator
                            );
    if (!SUCCEEDED(hres))
    {
        log_err "CoCreateInstance() failed. hres=%u", hres log_end;
        CoUninitialize();
        return false;
    }

    // #4, Connect to WMI through IWbemLocator::ConnectServer
    // 
    // Connect to the `root\cimv2` namespace with the current user and 
    // obtain pointer _svc to make IWbemServices calls.
    hres = _locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),     // Object path of WMI namespace
                               NULL,                        // User name. NULL = current user
                               NULL,                        // User password. NULL = current 
                               0,                           // Locale. NULL indicates current
                               NULL,                        // Serucirty flags.
                               0,                           // Authority (for example, Kerberos)
                               0,                           // Context object
                               &_svc                        // Pointer to IWbemServices proxy
                               );
    if (!SUCCEEDED(hres))
    {
        log_err "_locator->ConnectServer() failed. hres=%u", hres log_end;
        _locator->Release(); _locator = NULL;
        CoUninitialize();
        return false;
    }

    log_dbg "Connected to ROOT\\CIMV2 WMI name space." log_end;

    // #5, Security levels on the proxy
    hres = CoSetProxyBlanket(_svc,                          // Indicates the proxy to set
                             RPC_C_AUTHN_WINNT,             // RPC_C_AUTHN_XXX
                             RPC_C_AUTHZ_NONE,              // RPC_C_AUTHZ_XXX
                             NULL,                          // Server principal name
                             RPC_C_AUTHN_LEVEL_CALL,        // RPC_C_AUTHN_LEVEL_XXX
                             RPC_C_IMP_LEVEL_IMPERSONATE,   // RPC_C_IMP_LEVEL_XXX
                             NULL,                          // client identity
                             EOAC_NONE                      // proxy capabilities
                             );
    if (!SUCCEEDED(hres))
    {
        log_err "CoSetProxyBlanket() failed. hres=%u", hres log_end;
        _svc->Release(); _svc = NULL;
        _locator->Release(); _locator = NULL;
        CoUninitialize();
        return false;
    }

    _initialized = true;
    return true;
}

void WmiClient::finalize()
{
    if (_initialized)
    {
        _ASSERTE(NULL != _locator);
        _ASSERTE(NULL != _svc);
        _svc->Release();
        _locator->Release();
        _svc = NULL;
        _locator = NULL;
        CoUninitialize();

        _initialized = false;
    }
}

bool WmiClient::query(_In_ const wchar_t* query, _Out_ IEnumWbemClassObject*& enumerator)
{
    _ASSERTE(true == _initialized);
    if (true != _initialized) return false;

    _ASSERTE(NULL != query);
    if (NULL == query) return false;

    // Use the IWbemServices pointer to make request of WMI
    HRESULT hres = _svc->ExecQuery(bstr_t("WQL"),
                                   bstr_t(query),
                                   WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                   NULL,
                                   &enumerator
                                   );
    if (!SUCCEEDED(hres))
    {
        log_err "_svc->ExecQuery() failed. hres=%u", hres log_end;
        return false;
    }

    return true;
}


bool test_wmi_client()
{
    WmiClient wc;
    if (!wc.initialize())
    {
        return false;
    }

    IEnumWbemClassObject* enumerator = NULL;
    if (!wc.query(L"select * from Win32_LogicalDiskToPartition", enumerator))
    {
        return false;
    }

    while (enumerator)
    {
        IWbemClassObject *pclsObj = NULL;
        ULONG uReturn = 0;
        HRESULT hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        do
        {
            hr = pclsObj->Get(L"Antecedent", 0, &vtProp, 0, 0);
            if (!SUCCEEDED(hr))
            {
                log_err "pclsObj->Get() failed. hres=%u", hr log_end;
                break;
            }
            else
            {
                log_info "Antecedent=%ws", vtProp.bstrVal log_end;
                VariantClear(&vtProp);
            }

            hr = pclsObj->Get(L"Dependent", 0, &vtProp, 0, 0);
            if (!SUCCEEDED(hr))
            {
                log_err "pclsObj->Get() failed. hres=%u", hr log_end;
                break;
            }
            else
            {
                log_info "Dependent=%ws", vtProp.bstrVal log_end;
                VariantClear(&vtProp);
            }
        } while (false);
        
        pclsObj->Release();
    }
    enumerator->Release();


    wc.finalize();
    return true;
}