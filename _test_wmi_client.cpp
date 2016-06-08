/**
 * @file    wmi_client test
 * @brief   
 *
 * @author  Yonhgwhan, Noh (fixbrain@gmail.com)
 * @date    2016.06.08 15:36 created.
 * @copyright All rights reserved by Yonghwan, Noh.
**/
#include "stdafx.h"
#include "wmi_client.h"


/// @brief  send wmi query 
/// @param  query   query string to send
///         e.g. 
///             SELECT * FROM Win32_DiskDrive
///             SELECT Index, PNPDeviceID, SerialNumber FROM Win32_DiskDrive    
bool query_wmi(_In_ const wchar_t* query)
{
    _ASSERTE(NULL != query);
    if (NULL == query) return false;

    WmiClient   wmi;
    if (true != wmi.initialize())
    {
        log_err "WmiClient::initialize() failed." log_end;
        return false;
    }
    
    IEnumWbemClassObject* enumerator = NULL;
    if (!wmi.query(query, enumerator))
    {
        log_err "query failed. query=%s", query log_end;
        return false;
    }

    log_info "query=%ws", query log_end;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
    for (;;)
    {
        HRESULT hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (!SUCCEEDED(hr)) break;
        if (uReturn != 1)   break;

        // current object 의 모든 qualifier 를 가져온다. 
        SAFEARRAY* names = NULL;
        hr = pclsObj->GetNames(NULL, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, NULL, &names);
        if (!SUCCEEDED(hr))
        {
            log_err "IWbemClassObject::GetNames() failed." log_end;
            break;
        }

        // names 의 min/max 를 구한다. 
        long lbound = 0, ubound = 0;
        hr = SafeArrayGetLBound(names, 1, &lbound);
        hr = SafeArrayGetUBound(names, 1, &ubound);

        // names 를 순회하면서 각 property 의 값을 가져온다. 
        for (long i = lbound; i <= ubound; ++i)
        {
            wchar_t* name = NULL;
            hr = SafeArrayGetElement(names, &i, &name);
            if (!SUCCEEDED(hr))
            {
                log_err "SafeArrayGetElement( index=%u ) failed.", i log_end;
                continue;
            }

            VARIANT obj_prop_type;
            VARIANT obj_prop_value;
            VariantClear(&obj_prop_type);
            VariantClear(&obj_prop_value);

            // 현재 property 의 타입/값을 구한다. 시스템 속성인 경우 타입셋이 존재하지 않는다. 
            // 참고: http://serious-code.net/doku/doku.php?id=kb:wmiusingcpp
            IWbemQualifierSet* objProperties = NULL;
            hr = pclsObj->GetPropertyQualifierSet(name, &objProperties);
            if (!SUCCEEDED(hr))
            {
                log_err "IWbemClassObject::GetPropertyQualifierSet( property name=%ws ) failed.", name log_end;
                continue;
            }


            // objProperties 에는 어떤 프로퍼티들이 있나 출력해본다. 
            // => CIMTYPE 밖에 없다. 가끔 key 도 있긴하지만.
            // => 참고) Standard WMI Qualifiers, https://msdn.microsoft.com/en-us/library/aa393651(v=vs.85).aspx
            // 
            //{
            //    // objProperties 의 name 을 몽땅 가져와보자.
            //    SAFEARRAY* names2 = NULL;
            //    hr = objProperties->GetNames(0, &names2);
            //    if (!SUCCEEDED(hr))
            //    {
            //        log_err "IWbemQualifierSet::GetNames() failed." log_end;
            //        break;
            //    }
            //
            //    // names 의 min/max 를 구한다. 
            //    long lbound2 = 0, ubound2 = 0;
            //    hr = SafeArrayGetLBound(names2, 1, &lbound2);
            //    hr = SafeArrayGetUBound(names2, 1, &ubound2);
            //    
            //    for (long j = lbound2; j <= ubound2; ++j)
            //    {
            //        wchar_t* name2 = NULL;
            //        SafeArrayGetElement(names2, &j, &name2);
            //        log_info "name=%ws", name2 log_end;
            //    }
            //    SafeArrayDestroy(names2);
            //}

            hr = objProperties->Get(L"CIMTYPE", 0, &obj_prop_type, NULL);
            if (!SUCCEEDED(hr))
            {
                log_err "IWbemQualifierSet::Get( property name=CIMTYPE ) failed." log_end;
                continue;
            }

            // value 가 없는 경우도 있다. 
            hr = pclsObj->Get(name, 0, &obj_prop_value, 0, 0);
            if (!SUCCEEDED(hr))
            {
                log_err "pclsObj->Get( property=%ws) failed. hres=%u", name, hr log_end;
                continue;
            }

            log_info "    property name=%ws, type=%ws, value=(%u) %s",
                name,
                obj_prop_type.bstrVal,
                obj_prop_type.vt,
                (VT_NULL == obj_prop_value.vt || VT_EMPTY == obj_prop_value.vt) 
                    ? "NULL" 
                    : variant_to_str(obj_prop_value).c_str()
                log_end;
            
            objProperties->Release();
        }
        
        
        SafeArrayDestroy(names);
        pclsObj->Release();
    }
    enumerator->Release();
    return true;
}



bool test_wmi_client()
{
    const wchar_t* queries[] = {
        L"SELECT * FROM Win32_DiskDrive",
        L"SELECT Index, PNPDeviceID, SerialNumber FROM Win32_DiskDrive"
    };

    

    for (int i = 0; i < sizeof(queries) / sizeof(wchar_t*); ++i)
    {
        if (true != query_wmi(queries[i]))
        {
            log_err "query_wmi( query=%ws ) failed.", queries[i] log_end;
            continue;
        }

        log_info "query=%ws, succeeded.", queries[i] log_end;
    }

    return true;
}
