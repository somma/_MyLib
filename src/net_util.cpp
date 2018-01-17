/**
 * @file    Network configuration related codes based on IPHelp api
 * @brief   
 * @ref     https://msdn.microsoft.com/en-us/library/windows/desktop/aa365819(v=vs.85).aspx
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2017/12/29 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "net_util.h"
#include "log.h"


///////////////////////////////////////////////////////////////////////////////
//
//	NetAdapter class
//
///////////////////////////////////////////////////////////////////////////////

void NetAdapter::dump()
{
#ifdef _DEBUG
	std::stringstream strm;

	strm << WcsToMbsEx(friendly_name.c_str()) << std::endl;
	strm << "name=" << name << std::endl;
	strm << "desc=" << WcsToMbsEx(desc.c_str()) << std::endl;
	strm << "physical=" << physical_address << std::endl;
	
	if (ip_info_list.size() > 1)
	{
		strm << "ip=" << std::endl;
		for (auto ip_info : ip_info_list)
		{
			strm << "\t" << ip_info->ip << std::endl;
		}
	}
	else if (ip_info_list.size() == 1)
	{
		strm << "ip=" << ip_info_list[0]->ip.c_str() << std::endl;
	}
	
	if (dns_list.size() > 1)
	{
		strm << "dns=" << std::endl;
		for (auto dns : dns_list)
		{
			strm << "\t" << dns << std::endl;
		}
	}
	else if (dns_list.size() == 1)
	{
		strm << "dns=" << dns_list[0].c_str() << std::endl;
	}

	if (gateway_list.size() > 1)
	{
		strm << "gateway=" << std::endl;
		for (auto gw : gateway_list)
		{
			strm << "\t" << gw << std::endl;
		}
	}
	else if (gateway_list.size() == 1)
	{
		strm << "gateway=" << gateway_list[0].c_str() << std::endl;
	}

	log_dbg "%s", strm.str().c_str() log_end;

#endif//_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
//
//	Utility function 
//
///////////////////////////////////////////////////////////////////////////////


/// @brief	Read host name of this computer
bool 
get_host_name(
	_Out_ std::wstring& host_name
	)
{
	DWORD length = 0;
	if (GetComputerNameExW(ComputerNameNetBIOS,nullptr,&length) ||
		ERROR_MORE_DATA != GetLastError())
	{
		_ASSERTE(!"oops");
		return false;
	}

	wchar_ptr buf((wchar_t*)malloc(length * sizeof(wchar_t)), [](wchar_t* p) 
	{
		if (nullptr != p) free(p);
	});
	if (nullptr == buf.get())
	{
		log_err "Not enough memory." log_end;
		return false;
	}

	if (!GetComputerNameExW(ComputerNameNetBIOS,
							buf.get(),
							&length))
	{
		log_err "GetComputerNameExW() failed, gle = %u", 
			GetLastError() 
			log_end
		return false;
	}

	/// Enforce null terminate
	buf.get()[length] = 0x00;
	host_name = buf.get();
	return true;
}

/// @brief	Read network adapter information
///	@param	net_family	AF_INET
bool 
get_net_adapters(
	_In_ ULONG net_family, 
	_Out_ std::vector<PNetAdapter>& adapters
	)
{
	if ((net_family != AF_INET) && (net_family != AF_INET6))
	{
		log_err "Invalid net family. Only AF_INET(2), AF_INET(23) supported."
			log_end;
		return false;
	}

	//
	//	flag 가 0 일때 unicast, anycast, multicast 정보를 가져온다. 
	//	여기에 include, exclude flag 를 조합해서 사용할 수 있게 한것 같은데, 
	//	include, exclude 가 섞이면 뭔가 생각한 대로 결과가 안나옴
	//

	ULONG flags = GAA_FLAG_INCLUDE_GATEWAYS;
	ULONG size = 0;
	PIP_ADAPTER_ADDRESSES address = nullptr;	
	ULONG ret = GetAdaptersAddresses(net_family, 
									 flags, 
									 nullptr, 
									 address, 
									 &size);
	if (ERROR_BUFFER_OVERFLOW != ret || 0 == size)
	{
		log_err "GetAdapterAddress() failed. ret=%u, ERROR_BUFFER_OVERFLOW expected",
			ret
			log_end;
		return false;
	}

	char_ptr cptr((char*)malloc(size), [](char* p) {if (nullptr != p) free(p); });
	if (nullptr == cptr.get())
	{
		log_err "Not enough memory" log_end;
		return false;
	}
	address = (PIP_ADAPTER_ADDRESSES)cptr.get();

	ret = GetAdaptersAddresses(net_family,
							   flags,
							   nullptr,
							   address,
							   &size);
	if (ERROR_SUCCESS != ret)
	{
		log_err "GetAdapterAddress() failed. ret=%u",
			ret
			log_end;
		return false;
	}

	//
	//	Iterate all addresses
	//
	std::string str;
	PIP_ADAPTER_ADDRESSES cur = address;
	while (nullptr != cur)
	{
		///	IF_TYPE_SOFTWARE_LOOPBACK 인터페이스 정보는 제외
		if (cur->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
		{
			goto _next;
		}
		
		///	현재 활성화 되어있지 않는 어댑터 정보는 수집하지 않는다. 
		if (IfOperStatusUp != cur->OperStatus)
		{
			goto _next;
		}
	
		//
		//	NetAdapter 객체를 생성하고, 리스트에 추가한다. 
		//

		PNetAdapter adapter = new NetAdapter();
		if (nullptr == adapter)
		{
			log_err "Not enough memory. " log_end;
			return false;
		}
		adapters.push_back(adapter);
		
		///	Friendly name
		if (nullptr != cur->FriendlyName)
		{
			adapter->friendly_name = cur->FriendlyName;
		}

		///	Adapter name  
		if (nullptr != cur->AdapterName)
		{
			adapter->name = cur->AdapterName;
		}

		///	Adapter desc
		if (nullptr != cur->Description)
		{
			adapter->desc = cur->Description;
		}

		//	Physical address
		//	data link layer 인터페이스가 아닌 경우 0 일 수 있다. 
		if (cur->PhysicalAddressLength > 0)
		{
			const char* table = get_int_to_char_table(true);
			char buf[MAX_ADAPTER_ADDRESS_LENGTH * 3] = { 0x00 };
			ULONG buf_pos = 0;
			for (ULONG i = 0; i < cur->PhysicalAddressLength; ++i)
			{
				if (buf_pos > 0) { buf[buf_pos++] = '-'; }

				buf[buf_pos++] = table[cur->PhysicalAddress[i] >> 4];
				buf[buf_pos++] = table[cur->PhysicalAddress[i] & 0xf];
			}
			buf[buf_pos] = 0x00;
			adapter->physical_address = buf;
		}

		///	Assigned IP
		ULONG subnet_mask;
		PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = cur->FirstUnicastAddress;
		while (nullptr != unicast_addr)
		{
			if (true == SocketAddressToStr(&unicast_addr->Address, str))
			{
				//
				//	subnet mask
				//	PrefixLength를 이용해서 subnet mask를 구한다.
				//
				if (NO_ERROR == ConvertLengthToIpv4Mask(unicast_addr->OnLinkPrefixLength, &subnet_mask))
				{
					PNetIpInfo ip_info = new NetIpInfo;
					ip_info->ip = str;
					ip_info->subnet_mask = subnet_mask;
					adapter->ip_info_list.push_back(ip_info);
				}
			}

			unicast_addr = unicast_addr->Next;
		}

		/////	Anycast IP
		//PIP_ADAPTER_ANYCAST_ADDRESS anycast_addr = cur->FirstAnycastAddress;
		//while (nullptr != anycast_addr)
		//{
		//	if (true == SocketAddressToStr(&anycast_addr->Address, str))
		//	{
		//		log_info "anycst=%s", str.c_str() log_end;
		//	}
		//	anycast_addr = anycast_addr->Next;
		//}

		/////	Multicast IP
		//PIP_ADAPTER_MULTICAST_ADDRESS multicast_addr = cur->FirstMulticastAddress;
		//while (nullptr != multicast_addr)
		//{
		//	if (true != SocketAddressToStr(&multicast_addr->Address, str))
		//	{
		//		log_info "multicast=%s", str.c_str() log_end;
		//	}
		//	multicast_addr = multicast_addr->Next;
		//}

		///	DNS servers 
		PIP_ADAPTER_DNS_SERVER_ADDRESS dns = cur->FirstDnsServerAddress;
		while (nullptr != dns)
		{
			if (true == SocketAddressToStr(&dns->Address, str))
			{
				adapter->dns_list.push_back(str);
			}
			dns = dns->Next;
		}

		///	Gateway
		PIP_ADAPTER_GATEWAY_ADDRESS gateway = cur->FirstGatewayAddress;
		while (nullptr != gateway)
		{
			if (true == SocketAddressToStr(&gateway->Address, str))
			{
				adapter->gateway_list.push_back(str);
			}
			gateway = gateway->Next;
		}

	_next: 
		cur = cur->Next;
	}
	
	return true;
}

/// @brief	Socket address to string
bool
SocketAddressToStr(
	_In_ const SOCKET_ADDRESS* addr,
	_Out_ std::string& addr_str
	)
{
	_ASSERTE(nullptr != addr);
	if (nullptr == addr) return false;

	return SocketAddressToStr(addr->lpSockaddr, addr_str);
}

/// @brief	Winsock 을 초기화한다.
bool
InitializeWinsock(
	)
{
	static bool initialized = false;
	if (false == initialized)
	{
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		DWORD err = WSAStartup(wVersionRequested, &wsaData);
		if (0 != err)
		{
			_ASSERTE(!"Oops! Check OS version  plz.");
			log_err "WSAStartup() failed, err=%u",
				err
				log_end;
			return false;
		}
	}

	return true;
}

/// @brief	Socket address to string
///	@remark	WSAAddressToStringA 함수는 deprecated 되었기 때문에 W 버전 함수를 사용하고, 
///			address string 문자열을 변환한다.
bool 
SocketAddressToStr(
	_In_ const SOCKADDR* addr, 
	_Out_ std::string& addr_str
	)
{
	_ASSERTE(nullptr != addr);
	if (nullptr == addr) return false;

	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return false;

	//
	//	필요한 사이즈를 계산
	//
	DWORD buf_len = 0;	
	int ret = WSAAddressToStringW(
					(LPSOCKADDR)addr,
					(addr->sa_family == AF_INET) ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6),
					nullptr,
					nullptr,
					&buf_len);
	if (SOCKET_ERROR != ret || WSAEFAULT != WSAGetLastError())
	{
		log_err "Oops! Invalid status" log_end;
		return false;
	}

	//
	//	버퍼 할당
	//
	_ASSERTE(buf_len > 0);
	wchar_ptr buf((wchar_t*)malloc( (buf_len + 1) * sizeof(wchar_t) ), [](wchar_t* p)
	{
		if (nullptr != p) free(p);
	});
	if (nullptr == buf.get())
	{
		log_err "Not enough memory. bytes=%u",
			buf_len
			log_end;
		return false;
	}

	//
	//	변환시도
	//
	ret = WSAAddressToStringW(
				(LPSOCKADDR)addr,
				(addr->sa_family == AF_INET) ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6),
				nullptr,
				buf.get(),
				&buf_len);
	if (SOCKET_ERROR == ret)
	{
		log_err "WSAAddressToStringW() failed. wsa gle=%u",
			WSAGetLastError()
			log_end;
		return false;
	}

	//
	//	wchar -> char 로 변환하고, 리턴
	//
	addr_str = WcsToMbsEx(buf.get());
	return true;
}

///	@brief	
bool
dns_query(
	_In_ uint32_t ip_netbyte_order,
	_In_ bool cache_only,
	_Out_ std::wstring& domain_name
	)
{
	std::string dns_query_ip;
	PDNS_RECORD dns_record;

	dns_query_ip = ipv4_to_str(swap_endian_32(ip_netbyte_order)).append(".IN-ADDR.ARPA");
	if (ERROR_SUCCESS != DnsQuery_W(MbsToWcsEx(dns_query_ip.c_str()).c_str(),
									DNS_TYPE_PTR, 
									(true == cache_only) ? DNS_QUERY_NO_WIRE_QUERY : DNS_QUERY_STANDARD,
									NULL, 
									&dns_record, 
									NULL))
	{
		log_warn "DnsQuery( ) failed. gle=%s",
			dns_query_ip.c_str()
			log_end;

		domain_name = L"";
		return false;
	}

	domain_name = dns_record->Data.PTR.pNameHost;
	return true;
}

///	@brief	
std::string 
ipv4_to_str(
	_In_ uint32_t ip_netbyte_order
	)
{
	in_addr ia;
	ia.s_addr = ip_netbyte_order;
	return ipv4_to_str(ia);
}

/// @brief  
std::string 
ipv4_to_str(
	_In_ in_addr& ipv4
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return std::string("0.0.0.0");

    char ipv4_buf[16 + 1] = { 0 };
    if (NULL == InetNtopA(AF_INET, 
						  &ipv4, 
						  ipv4_buf, 
						  sizeof(ipv4_buf)))
    {
        log_err "InetNtopA( ) failed. wsa_gle = %u", 
			WSAGetLastError() 
			log_end;
        return std::string("0.0.0.0");
    }
    return std::string(ipv4_buf);
}

/// @brief  
std::string 
ipv6_to_str(
	_In_ in_addr6& ipv6
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return std::string("0.0.0.0");

    char ipv6_buf[46 + 1] = { 0 };
    if (NULL == InetNtopA(AF_INET6, 
						  &ipv6, 
						  ipv6_buf, 
						  sizeof(ipv6_buf)))
    {
        log_err "InetNtopA( ) failed. wsa_gle = %u", 
			WSAGetLastError() 
			log_end;
        return std::string("0.0.0.0");
    }
    return std::string(ipv6_buf);
}

/// @brief  
bool 
str_to_ipv4(
	_In_ const wchar_t* ipv4, 
	_Out_ uint32_t& ip_netbyte_order
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return false;

    _ASSERTE(NULL != ipv4);
    if (NULL != ipv4)
    {
		in_addr ipv4_addr = { 0 };
        int ret = InetPtonW(AF_INET, 
							ipv4, 
							&ipv4_addr);
        switch (ret)
        {
        case 1: 
			ip_netbyte_order = ipv4_addr.s_addr;
            return true;    // success
        case 0:
            log_err "invalid ipv4 string. input = %ws", 
				ipv4 
				log_end;
            return false;
        case -1: 
            log_err "InetPtonW() failed. input = %ws, wsa gle = %u", 
				ipv4, 
				WSAGetLastError() 
				log_end;
            return false;
        }
    }

    return false;
}

/// @brief  
bool 
str_to_ipv6(
	_In_ const wchar_t* ipv6, 
	_Out_ in_addr6& ip_netbyte_order
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return false;

    _ASSERTE(NULL != ipv6);
    if (NULL != ipv6)
    {
		int ret = InetPtonW(AF_INET6, ipv6, &ip_netbyte_order);
        switch (ret)
        {
        case 1:			
            return true;    // success
        case 0:
            log_err "invalid ipv4 string. input = %ws", 
				ipv6 
				log_end;
            return false;
        case -1:
            log_err "InetPtonW() failed. input = %ws, wsa gle = %u", 
				ipv6, 
				WSAGetLastError() 
				log_end;
            return false;
        }
    }

    return false;
}

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool 
get_ip_list_v4(
	_Out_ std::vector<std::string>& ip_list
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return false;

	std::vector<PNetAdapter> adapters;
	if (true != get_net_adapters(AF_INET, adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return false;
	}

	for (auto adapter : adapters)
	{
		for (auto ip_info : adapter->ip_info_list)
		{
			ip_list.push_back(ip_info->ip);
			delete ip_info;
		}
		delete adapter;
	}
	adapters.clear();

	return true;
}

/**
* @brief
* @param
* @see
* @remarks
* @code
* @endcode
* @return
**/
bool
get_broadcast_list_v4(
	_Out_ std::vector<uint32_t>& broadcast_list
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return false;

	std::vector<PNetAdapter> adapters;
	if (true != get_net_adapters(AF_INET, adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return false;
	}

	for (auto adapter : adapters)
	{
		for (auto ip_info : adapter->ip_info_list)
		{
			uint32_t ip_addr = 0;
			if (true == str_to_ipv4(MbsToWcsEx(ip_info->ip.c_str()).c_str(), ip_addr))
			{
				broadcast_list.push_back(ip_addr | ~ip_info->subnet_mask);
			}
			delete ip_info;
		}
		delete adapter;
	}
	adapters.clear();

	return true;
}

/// @brief	Localhost 의 대표(?) ip 를 리턴한다. 
std::string
get_representative_ip_v4(
	)
{
	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return std::string("0.0.0.0");

	//
	//	1순위: 
	//		dns, gateway 가 설정된 어댑터의 첫번째 IP
	//	2순위:
	//		gateway 가 설정된 어댑터의 첫번째 IP
	//	3순위: 
	//		첫번째 IP
	// 
	std::vector<PNetAdapter> adapters;
	if (true != get_net_adapters(AF_INET, adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return "127.0.0.1";
	}

	std::string ip; 
	do
	{
		//
		//	1순위 찾기
		//
		for (auto adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			if (true != adapter->gateway_list.empty())
			{
				if (true != adapter->dns_list.empty())
				{
					ip = adapter->ip_info_list[0]->ip;
					break;
				}
			}
		}
		if (true != ip.empty()) break;

		//
		//	2순위 찾기
		//
		for (auto adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			if (true != adapter->gateway_list.empty())
			{
				ip = adapter->ip_info_list[0]->ip;
				break;
			}
		}
		if (true != ip.empty()) break;

		//
		//	3순위
		//
		for (auto adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			ip = adapter->ip_info_list[0]->ip;
			break;
		}

	} while (false);

	//
	//	Free
	//	
	std::for_each(adapters.begin(), adapters.end(), [](PNetAdapter p) 
	{
		if (nullptr != p)
		{
			for (auto ip_info : p->ip_info_list)
			{
				delete ip_info;
			}

			delete p;
		}
	});
	adapters.clear();

	return (true != ip.empty()) ? ip : "127.0.0.1";
}

/// @brief  `ip_str` 주소를 가진 interface 의 mac 주소를 알아낸다. 
std::string
get_mac_by_ip_v4(
	_In_ const char* ip_str	
	)
{
	_ASSERTE(NULL != ip_str);
    if (NULL == ip_str) return false;

	//
	//	Winsock 초기화
	//
	if (!InitializeWinsock()) return std::string("0.0.0.0");

	//
	//	어댑터 정보를 가져온다.
	//
	std::vector<PNetAdapter> adapters;
	if (true != get_net_adapters(AF_INET, adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return "00:00:00:00:00:00";
	}

	//
	//	IP 와 매칭한다.
	//
	bool matched = false;
	std::string mac;
	for (auto adapter : adapters)
	{
		if (true == adapter->ip_info_list.empty()) continue;
		for (auto ip_info : adapter->ip_info_list)
		{
			if (0 == ip_info->ip.compare(ip_str))
			{
				mac = adapter->physical_address;
				matched = true;
				break;
			}
		}		

		if (true == matched) break;
	}

	//
	//	Free
	//
	std::for_each(adapters.begin(), adapters.end(), [](PNetAdapter p)
	{
		if (nullptr != p)
		{
			for (auto ip_info : p->ip_info_list)
			{
				delete ip_info;
			}
			delete p;
		}
	});
	adapters.clear();

	return (true == mac.empty()) ? "00:00:00:00:00:00" : mac;
}









