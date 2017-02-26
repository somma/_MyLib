/**
 * @file    
 * @brief   
 * 
 * This file contains 
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2016:06:04 10:02 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#pragma once

#define _WIN32_DCOM
#include <comdef.h>
#include <WbemIdl.h>
#include <WbemCli.h>
#pragma comment(lib, "WbemUuid.lib")

typedef class WmiClient
{
public:
    WmiClient();
    ~WmiClient();

    bool initialize();
    void finalize();

    bool query(_In_ const wchar_t* query, _Out_ IEnumWbemClassObject*& enumerator);
private:
    bool _initialized;

    IWbemLocator*   _locator;
    IWbemServices*  _svc;
} *PWmiClient;


std::string variant_to_str(const VARIANT& var);


