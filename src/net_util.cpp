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
	
	if (ip_list.size() > 1)
	{
		strm << "ip=" << std::endl;
		for (auto ip : ip_list)
		{
			strm << "\t" << ip << std::endl;
		}
	}
	else if (ip_list.size() == 1)
	{
		strm << "ip=" << ip_list[0].c_str() << std::endl;
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
//	NetConfig class
//
///////////////////////////////////////////////////////////////////////////////

NetConfig::~NetConfig()
{
	for (auto adapter : _adapters)
	{
		delete adapter;
	}
	_adapters.clear();
}



/// @brief	
bool NetConfig::read_net_config()
{
	//
	//	host name
	//
	if (true != get_host_name(_host_name))
	{
		log_err "get_host_name() failed. " log_end;
		return false;
	}

	//
	//	Network adapters (IPV4 만 가져온다)
	//
	if (true != get_net_adapters(AF_INET, _adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return false;
	}

	return true;
}

/// @brief
void NetConfig::dump()
{
#ifdef _DEBUG
	log_dbg "host=%s", WcsToMbsEx(_host_name.c_str()).c_str() log_end;
	for (auto adapter : _adapters)
	{
		adapter->dump();
	}
#endif
}


/// @brief	Read host name of this computer
bool NetConfig::get_host_name(_Out_ std::wstring& host_name)
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
NetConfig::get_net_adapters(
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
		PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = cur->FirstUnicastAddress;
		while (nullptr != unicast_addr)
		{
			if (true == SocketAddressToStr(&unicast_addr->Address, str))
			{
				adapter->ip_list.push_back(str);
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





///////////////////////////////////////////////////////////////////////////////
//
//	Utility function 
//
///////////////////////////////////////////////////////////////////////////////


#pragma todo("Win32Util 에서 winsock 관련 코드는 여기로 가져오기 ")

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


/// @brief	Socket address to string
bool 
SocketAddressToStr(
	_In_ const SOCKADDR* addr, 
	_Out_ std::string& addr_str
	)
{
	_ASSERTE(nullptr != addr);
	if (nullptr == addr) return false;
	
	//
	//	WSAAddressToStringA 함수는 deprecated 되었기 때문에
	//	W 버전 함수를 사용하고, address string 문자열을 변환한다.
	//
	DWORD buf_len = 0;	
	int ret = WSAAddressToStringW((LPSOCKADDR)addr,
								  (addr->sa_family == AF_INET) ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6),
								  nullptr,
								  nullptr,
								  &buf_len);
	if (SOCKET_ERROR != ret || WSAEFAULT != WSAGetLastError())
	{
		log_err "Oops! Invalid status" log_end;
		return false;
	}

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

	ret = WSAAddressToStringW((LPSOCKADDR)addr,
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
	//	wchar -> char 로 변환
	//
	addr_str = WcsToMbsEx(buf.get());
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
ipv6_to_str(
	_In_ uint64_t ip_netbyte_order
	)
{
	in_addr6 ia;
	RtlCopyMemory(ia.s6_addr, 
				  &ip_netbyte_order, 
				  sizeof(ip_netbyte_order));
	return ipv6_to_str(ia);
}

/// @brief  
std::string 
ipv4_to_str(
	_In_ in_addr& ipv4
	)
{
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
	_In_ in6_addr& ipv6
	)
{
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
	_Out_ in_addr& ipv4_addr
	)
{
    _ASSERTE(NULL != ipv4);
    if (NULL != ipv4)
    {
        int ret = InetPtonW(AF_INET, 
							ipv4, 
							&ipv4_addr);
        switch (ret)
        {
        case 1: 
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
	_Out_ in6_addr& ipv6_addr
	)
{
    _ASSERTE(NULL != ipv6);
    if (NULL != ipv6)
    {
        int ret = InetPtonW(AF_INET6, ipv6, &ipv6_addr);
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

/// @brief  
bool 
get_ip_by_hostname(
	_In_ const wchar_t* host_name, 
	_Out_ std::wstring& ip_string
	)
{
    _ASSERTE(NULL != host_name);
    if (NULL == host_name) return false;

    bool ret = false;

    ADDRINFOW hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    do
    {
        PADDRINFOW Result = NULL;
        if (0 != GetAddrInfoW(host_name, 
							  NULL, 
							  &hints, 
							  &Result) || 
			NULL == Result)
        {
            log_err 
                "GetAddrInfoW(host_name = %s) failed, wserr=0x%08x", 
                host_name, 
                WSAGetLastError() log_end;
            break;
        }

        for (PADDRINFOW p = Result; p != NULL; p = p->ai_next)
        {
            if (AF_INET == p->ai_family)
            {
                sockaddr_in* sa = (sockaddr_in*)p->ai_addr;

#if (NTDDI_VERSION >= NTDDI_VISTA)
                wchar_t addr[16] = { 0 };
                if (NULL == InetNtop(AF_INET, (PVOID)&sa->sin_addr, addr, 16))
                {
                    log_err "InetNtop() failed. gle = %u", WSAGetLastError() log_end;
                    continue;
                }
                ip_string = addr;                
#else
                ip_string = MbsToWcsEx(inet_ntoa(sa->sin_addr)));
#endif

                ret = true;
                break;
            }
        }
        
        FreeAddrInfoW(Result);    
    } while (false);
    WSACleanup();

    return ret;
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
get_local_ip_list(
	_Out_ std::wstring& host_name, 
	_Out_ std::vector<std::string>& ip_list
	)
{
	WSADATA wsaData={0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	DWORD NetbiosNameLen = 0;
	wchar_t* netbios_name = NULL;

	if(0 == GetComputerNameExW(ComputerNameNetBIOS, 
							   netbios_name, 
							   &NetbiosNameLen))
	{
		if(ERROR_MORE_DATA == GetLastError())
		{
			// GetComputerNameExW() 가 리턴하는 NetbiosNameLen 은 null 을 포함한다.
			netbios_name = (wchar_t*) malloc(NetbiosNameLen * sizeof(wchar_t));
			if (NULL == netbios_name) return false;

			if(0 == GetComputerNameExW(ComputerNameNetBIOS, 
									   netbios_name, 
									   &NetbiosNameLen))
			{
				log_err "GetComputerNameExW( ComputerNameNetBIOS ) failed, gle = %u", GetLastError() log_end

				free(netbios_name); netbios_name = NULL;
				WSACleanup();
				return false;
			}
			else
			{
				// enforce null terminate string.
				netbios_name[NetbiosNameLen] = 0x00;
			}
		}
	}

	host_name = netbios_name;
	free(netbios_name); netbios_name = NULL;

	ADDRINFOW hints={0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	PADDRINFOW Result=NULL;
	if( 0 != GetAddrInfoW(host_name.c_str(), NULL, &hints, &Result) || NULL == Result )
	{
		log_err "GetAddrInfoW(host_name = %s) failed, wserr=0x%08x", host_name.c_str(), WSAGetLastError() log_end
		WSACleanup();
		return false;
	}

	for (PADDRINFOW p = Result; p != NULL; p = p->ai_next)
	{
		if (AF_INET == p->ai_family)
		{
            sockaddr_in* sa = (sockaddr_in*) p->ai_addr;			

#if (NTDDI_VERSION >= NTDDI_VISTA)
            char addr[16] = {0};
            if (NULL == InetNtopA(AF_INET, (PVOID)&sa->sin_addr, addr, 16))
            {
                log_err "InetNtopA() failed. gle = %u", WSAGetLastError() log_end
                continue;
            }            
            ip_list.push_back( addr );
#else
            ip_list.push_back( inet_ntoa(sa->sin_addr) );
#endif
		}            
	}

	FreeAddrInfoW(Result); 
	WSACleanup();
	return true;
}

/// @brief  `ip_str` 주소를 가진 interface 의 mac 주소를 알아낸다. 
///         리모트 시스템의 mac 주소를 알아오려는 목적인 아니고, 로컬 시스템에 연결된
///         interface 의 맥 주소를 알아오는 용도의 함수다. 
///         `127.0.0.1` 은 안됨
/// 
///         GetAdaptersAddresses() 함수를 사용할 수도 있으나 이게 더 간편해서 ...
///         cf. IP_ADAPTER_ADDRESSES  
///         https://msdn.microsoft.com/en-us/library/windows/desktop/aa366058(v=vs.85).aspx
/// 
/// @param  ip_str
bool 
get_local_mac_by_ipv4(
	_In_ const char* ip_str, 
	_Out_ std::string& mac_str
	)
{
    _ASSERTE(NULL != ip_str);
    if (NULL == ip_str) return false;

    IN_ADDR src = { 0 };
    if (0 == InetPtonA(AF_INET, ip_str, &src))
    {
        log_err "InetPtonA( %s ) failed.", 
			ip_str 
			log_end;
        return false;
    }

    uint8_t mac[8] = { 0 };
    uint32_t cb_mac = sizeof(mac);
    DWORD ret = SendARP((IPAddr)src.S_un.S_addr, (IPAddr)src.S_un.S_addr, mac, (PULONG)&cb_mac);
    if (NO_ERROR != ret)
    {
        log_err "SendARP( %s ) failed. ret = %u", 
			ip_str, 
			ret 
			log_end;
        return false;
    }

    char buf[18] = { 0 };   // 01-34-67-9a-cd-f0[null]

    StringCbPrintfA(&buf[0], 6, "%02x", mac[0]);
    StringCbPrintfA(&buf[2], 4, "-");

    StringCbPrintfA(&buf[3], 6, "%02x", mac[1]);
    StringCbPrintfA(&buf[5], 4, "-");

    StringCbPrintfA(&buf[6], 6, "%02x", mac[2]);
    StringCbPrintfA(&buf[8], 4, "-");

    StringCbPrintfA(&buf[9], 6, "%02x", mac[3]);
    StringCbPrintfA(&buf[11], 4, "-");

    StringCbPrintfA(&buf[12], 6, "%02x", mac[4]);
    StringCbPrintfA(&buf[14], 4, "-");

    StringCbPrintfA(&buf[15], 6, "%02x", mac[5]);
    
    mac_str = buf;
    return true;
}









