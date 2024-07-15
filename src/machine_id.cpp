/**
 * @file    machine_id.h
 * @brief	This module generates an almost unique hardware id.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    07.22.2022 15:13 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "log.h"
#include "machine_id.h"
#include "net_util.h"
#include "md5.h"


/// @brief	Generates almost unique id based on installed hardware.
///	@remark	ref: https://github.com/Tarik02/machineid/blob/master/src/machineid/machineid.cpp
bool generate_machine_id(_Out_ std::string& machine_id)
{
	if (!init_net_util())
	{
		log_err "init_net_util() failed." log_end;
		return false;
	}

	bool ret = false;
	std::stringstream strm;
	do
	{
		//
		//	mac address
		//
		std::list<PInetAdapter> adapters;
		if (!get_inet_adapters(adapters))
		{
			log_err "get_inet_adapters() failed." log_end;
			break;
		}

		for (const auto& adapter : adapters)
		{
			strm << mac_to_str(adapter->physical_address);
		}

		std::for_each(adapters.cbegin(),
					  adapters.cend(),
					  [](_In_ const PInetAdapter& p)
		{
			_ASSERTE(nullptr != p);
			if (nullptr != p)
			{
				delete p;
			}
		});
		adapters.clear();
		
		//
		//	volume serial number		
		//
		DWORD v_serial = 0xffffffff;
		if (!GetVolumeInformationA(nullptr,		// current directory
								   nullptr, 
								   0, 
								   &v_serial, 
								   nullptr,
								   nullptr, 
								   nullptr,
								   0))
		{
			log_err "GetVolumeInformationA() failed. gle=%u",
				GetLastError()
				log_end;
			break;
		}
		strm << v_serial;

		//
		//	CPUID information
		//
		int cpuid[4];
		__cpuid(cpuid, 0);
		std::string cpuidx;
		if (!bin_to_hexa(sizeof(cpuid), (const UINT8*)&cpuid, false, cpuidx))
		{
			log_err "bin_to_hexa() failed." log_end;
			break;
		}
		strm << cpuidx;

		//
		//	hash it!
		//
		MD5_CTX md5 = { 0 };
		MD5Init(&md5, 0);
		MD5Update(&md5, 
				  (unsigned char*)strm.str().c_str(),
				  static_cast<unsigned int>(strm.str().size()));
		MD5Final(&md5);
		if (!bin_to_hexa_fast(sizeof(md5.digest),
							  md5.digest,
							  false,
							  machine_id))
		{
			log_err "bin_to_hexa_fast() failed." log_end;
			break;
		}

		//
		//	Done!
		//
		ret = true;
	} while (false);
	
	cleanup_net_util();
	return ret;
}
