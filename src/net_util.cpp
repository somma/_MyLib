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










