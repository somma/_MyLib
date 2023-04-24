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


//
//	예약된 IP 주소 목록
//
//	Ref #1: Address Allocation for Private Internets, https://tools.ietf.org/html/rfc1918
//	Ref #2: https://github.com/google/ipaddr-py/blob/master/ipaddr.py
//	Ref #3: https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
//
//	Ref #2 코드를 기반으로 아래 목록 작성
//
static const struct net_and_mask 
{
	bool	 is_private;
	uint32_t netbyte_order_ip;
	uint32_t netbyte_order_mask;

} __reserved_net_and_masks[] = 
{
	{true, 0x0000000a, 0x00ffffff},	// 10.0.0.0/8,		Private-Use
	{true, 0x0000a8c0, 0x0000ffff},	// 192.168.0.0/16,	Private-Use
	{true, 0x000010ac, 0x00f0ffff},	// 172.16.0.0/12,	Private-Use

	{false, 0x000000e0, 0xf0ffffff},	// 224.0.0.0/4,	Multicast 
	//{false, 0xffffffff, 0x00000000},	// 255.255.255.255/32, Limited Broadcast(RFC8190, 919)
										// 그냥 0xfffffff 로 비교하면 됨

	{false, 0x0000007f, 0x00ffffff},	// 127.0.0.0/8, Loop back

	{false, 0x0000fea9, 0x0000ffff},	// 169.254.0.0/16, Link local (RFC 3927)
	{false, 0x00004064, 0x00fcffff},	// 100.64.0.0/10, Shared Address Space
	{false, 0x000000f0, 0xf0ffffff},	// 240.0.0.0/4, Reserved

	//{false, 0x006433c6, 0x000000ff},	// 198.51.100.0/24, Documentation, RFC5737
	//{false, 0x007100cb, 0x000000ff},	// 203.0.113.0/24, Documentation, RFC5737	

	//{false, 0x000012c6, 0x0080ffff},	// 198.18.0.0/15, Benchmarking(RFC2544)
	{false, 0x00000000, 0x00ffffff}	// 0.0.0.0/8, RFC1122
};

static const uint32_t __crnam = sizeof(__reserved_net_and_masks) / sizeof(net_and_mask);




/// @brief	Winsock 을 초기화한다.
///			
///			!!주의!!
///			WSAStartup() / WSACleanup() 의 호출횟수는 정확히 일치해야 한다. 
bool init_net_util()
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
	return true;
}

/// @brief	Winsock 을 종료한다. 
///			
///			!!주의!!
///			WSAStartup() / WSACleanup() 의 호출횟수는 정확히 일치해야 한다. 
void cleanup_net_util()
{
	WSACleanup();
}

/// @brief	Read host name of this computer
bool
get_host_name(
	_Out_ std::wstring& host_name
)
{
	DWORD length = 0;
	if (GetComputerNameExW(ComputerNameNetBIOS, nullptr, &length) ||
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
get_inet_adapters(	
	_Out_ std::list<PInetAdapter>& adapters
)
{
	//
	//	flag 가 0 일때 unicast, anycast, multicast 정보를 가져온다. 
	//	여기에 include, exclude flag 를 조합해서 사용할 수 있게 한것 같은데, 
	//	include, exclude 가 섞이면 뭔가 생각한 대로 결과가 안나옴
	//

	ULONG flags = GAA_FLAG_INCLUDE_GATEWAYS;
	ULONG size = 0;
	PIP_ADAPTER_ADDRESSES address = nullptr;
	ULONG ret = GetAdaptersAddresses(AF_INET,
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

	ret = GetAdaptersAddresses(AF_INET,
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
	PInetAdapter adapter = nullptr;
	ULONG subnet_mask = 0;
	PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = nullptr;
	PIP_ADAPTER_DNS_SERVER_ADDRESS dns = nullptr;
	PIP_ADAPTER_GATEWAY_ADDRESS gateway = nullptr;

	PIP_ADAPTER_ADDRESSES cur = address;
	while (nullptr != cur)
	{
		///	IF_TYPE_SOFTWARE_LOOPBACK 인터페이스 정보는 제외
		if (cur->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
		{
			goto _next;
		}

		//
		// 활성화되지 않은 인터페이스에 대한 정보도 수집한다. 
		//
		//if (IfOperStatusUp != cur->OperStatus)
		//{
		//	goto _next;
		//}

		//
		//	InetAdapter 객체를 생성하고, 리스트에 추가한다. 
		//
		adapter = new InetAdapter(cur->IfType);
		if (nullptr == adapter)
		{
			log_err "Not enough memory. " log_end;
			return false;
		}

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
		if (cur->PhysicalAddressLength == 6)
		{
			RtlCopyMemory(adapter->physical_address,
						  cur->PhysicalAddress,
						  6);
		}

		///	Assigned IP
		unicast_addr = cur->FirstUnicastAddress;
		while (nullptr != unicast_addr)
		{
			//
			//	subnet mask
			//	PrefixLength를 이용해서 subnet mask를 구한다.
			//
			_ASSERTE(AF_INET == unicast_addr->Address.lpSockaddr->sa_family);
			if (AF_INET != unicast_addr->Address.lpSockaddr->sa_family){continue;}

			if (NO_ERROR == ConvertLengthToIpv4Mask(unicast_addr->OnLinkPrefixLength, &subnet_mask))
			{
				PSOCKADDR_IN si = (PSOCKADDR_IN)unicast_addr->Address.lpSockaddr;
				
				adapter->ip_info_list.push_back(
					new Ipv4Info(si->sin_addr.s_addr,subnet_mask)
				);
			}

			unicast_addr = unicast_addr->Next;
		}

		///	DNS servers 
		dns = cur->FirstDnsServerAddress;
		while (nullptr != dns)
		{
			_ASSERTE(AF_INET == dns->Address.lpSockaddr->sa_family);
			if (AF_INET != dns->Address.lpSockaddr->sa_family) { continue; }

			PSOCKADDR_IN si = (PSOCKADDR_IN)dns->Address.lpSockaddr;
			adapter->dns_list.push_back(si->sin_addr.s_addr);

			dns = dns->Next;
		}

		///	Gateway
		gateway = cur->FirstGatewayAddress;
		while (nullptr != gateway)
		{
			_ASSERTE(AF_INET == gateway->Address.lpSockaddr->sa_family);
			if (AF_INET != gateway->Address.lpSockaddr->sa_family) { continue; }

			PSOCKADDR_IN si = (PSOCKADDR_IN)gateway->Address.lpSockaddr;
			adapter->gateway_list.push_back(si->sin_addr.s_addr);

			gateway = gateway->Next;
		}

		//
		//	어댑터 객체 추가
		//
		adapters.push_back(adapter);
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
	wchar_ptr buf((wchar_t*)malloc((buf_len + 1) * sizeof(wchar_t)), [](wchar_t* p)
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
/*
	수동으로 ip -> DNS 조회 테스트 해보기

	> nslookup
	> set type=PTR
	> set debug

	> 103.27.148.71
	Server:  google-public-dns-a.google.com
	Address : 8.8.8.8

	------------
	Got answer :
		HEADER:
		opcode = QUERY, id = 5, rcode = NXDOMAIN
		header flags : response, want recursion, recursion avail.
		questions = 1, answers = 0, authority records = 1, additional = 0

		QUESTIONS :
			71.148.27.103.in - addr.arpa, type = PTR, class = IN
			AUTHORITY RECORDS :
		->  103.in - addr.arpa
		ttl = 1798 (29 mins 58 secs)
		primary name server = ns1.apnic.net
		responsible mail addr = read-txt-record-of-zone-first-dns-admin.apnic.net
		serial = 36246
		refresh = 7200 (2 hours)
		retry = 1800 (30 mins)
		expire = 604800 (7 days)
		default TTL = 172800 (2 days)

	------------
		*** google-public-dns-a.google.com can't find 71.148.27.103.in-addr.arpa.: Non-existent domain



	> 103.243.220.231
	Server:  google-public-dns-a.google.com
	Address : 8.8.8.8

	------------
	Got answer :
	HEADER:
		opcode = QUERY, id = 6, rcode = NOERROR
		header flags : response, want recursion, recursion avail.
		questions = 1, answers = 1, authority records = 0, additional = 0

		QUESTIONS :
			231.220.243.103.in-addr.arpa, type=PTR, class=IN
			ANSWERS :
		->  231.220.243.103.in - addr.arpa
		name = 175.bm-nginx-loadbalancer.mgmt.sin1.adnexus.net
		ttl = 3004 (50 mins 4 secs)

	------------
	Non - authoritative answer :
	231.220.243.103.in-addr.arpa
		name = 175.bm-nginx-loadbalancer.mgmt.sin1.adnexus.net
		ttl = 3004 (50 mins 4 secs)

*/
bool
ip_to_dns(
	_In_ uint32_t ip_netbyte_order,
	_In_ bool cache_only,
	_Out_ std::string& domain_name
)
{
	std::string dns_query_ip;
	PDNS_RECORD dns_record = nullptr;

	//
	//	127.0.0.1 -> 1.0.0.127.IN-ADDR.ARPA
	//
	dns_query_ip = ipv4_to_str(swap_endian_32(ip_netbyte_order)).append(".IN-ADDR.ARPA");

	//-------------------------------------------------------------------------
	//
	// DNS_QUERY_STANDARD 옵션을 사용한 경우 
	//	- local cache -> query with UDP -> query with TCP (응답데이터가 잘린경우)
	//	- DNS_QUERY_NO_MULTICAST : DNS 서버에만 요청을 전송(음?)
	//	- DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE 옵션을 주면 tcp 를 통한 재시도를 
	//	  방지 함
	//
	//--------------------------------------------------------------------------------
	//
	//	DNS_QUERY_STANDARD vs DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE 성능 비교
	//	ipconfig /flushdns 명령으로 cache 를 비우고 188 개의 ip 를 dns 로 변환테스트
	//	
	//	DNS_QUERY_STANDARD 옵션 사용
	//		1st try) total=188, succ=70, cache=0,  wire=70, elapsed= 145830.484375 ms
	//		2nd try) total=188, succ=70, cache=57, wire=13, elapsed= 137857.656250 ms
	//	1st try 와 2nd try 간 차이가 크지 않다(10초). DNS 이름이 없는 IP 들이 대부분의 
	//	시간을 잡아먹기 때문
	// 
	//	DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE 옵션 사용
	//		1st try) total=188, succ=70, cache=0, wire=70, elapsed=   33709.382813 ms
	//		2nd try) total=188, succ=70, cache=69, wire=1, elapsed=   14285.799805 ms
	//	전체적으로 DNS_QUERY_STANDARD 보다 약 5배 빠르다. DNS 이름 없는 IP 조회 시간이 상대적으로
	//	빠르기 때문에 1st try, 2nd try 간 차이가 DNS_QUERY_STANDARD 보다 더 큼
	//
	//--------------------------------------------------------------------------------
	DNS_STATUS status = 
		DnsQuery_A(dns_query_ip.c_str(),
				   DNS_TYPE_PTR,
				   cache_only ? DNS_QUERY_NO_WIRE_QUERY : (DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE),
				   NULL,
				   &dns_record,
				   NULL);
	if (ERROR_SUCCESS != status)
	{
		if (DNS_ERROR_RECORD_DOES_NOT_EXIST == status || DNS_ERROR_RCODE_NAME_ERROR == status)
		{
			//
			//	dns 이름 없는 경우, ... 뭐 그런거다... 없다.
			//
		}
		else
		{
			log_dbg "DnsQuery(cache_only=%s) failed. ip=%s, status=%u",
				true == cache_only ? "O" : "X",
				dns_query_ip.c_str(),
				status
				log_end;
		}

		domain_name = "";
		return false;
	}
	_ASSERTE(nullptr != dns_record);
	if (nullptr == dns_record)
	{
		domain_name = "";
		return false;
	}	
	domain_name = (char*)dns_record->Data.pDataPtr;

	//
	//	Free memory allocated for DNS records 
	//
	DnsRecordListFree(dns_record, DnsFreeRecordListDeep);
	return true;
}

/// @brief	www.google.com -> 1.1.1.1 로 변환하는 함수
bool
dns_to_ip(
	_In_ const char* domain_name,
	_In_ bool cache_only,
	_Out_ std::list<uint32_t>& ip_list
)
{
	_ASSERTE(nullptr != domain_name);
	if (nullptr == domain_name) return false;

	std::string dns_query_ip;
	DNS_RECORDW* dns_record = nullptr;
	std::wstring dnsw = MbsToWcsEx(domain_name);

	DNS_STATUS status = 
		DnsQuery_W(dnsw.c_str(), 
				   DNS_TYPE_A,
				   (true == cache_only) ? 
						DNS_QUERY_NO_WIRE_QUERY : 
						(DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE),
				   NULL,
				   &dns_record,
				   NULL);
	if (ERROR_SUCCESS != status)
	{
		if (DNS_ERROR_RECORD_DOES_NOT_EXIST == status || 
			DNS_ERROR_RCODE_NAME_ERROR == status)
		{
			//
			//	유효하지 않은 도메인 네임
			//
			log_dbg "DnsQuery(cache_only=%s) failed. domain=%s, status=%u",
				true == cache_only ? "O" : "X",
				domain_name,
				status
				log_end;
		}
		else
		{
			log_dbg "DnsQuery(cache_only=%s) failed. ip=%s, status=%u",
				true == cache_only ? "O" : "X",
				domain_name,
				status
				log_end;
		}

		return false;
	}

	_ASSERTE(nullptr != dns_record);
	if (nullptr == dns_record)
	{
		log_err 
			"DnsQuery(cache_only=%s) succeeded but no recored. domain=%s",
			true == cache_only ? "O" : "X",
			domain_name
			log_end;

		return false;
	}
	
	while(dns_record != nullptr)
	{
		if (dns_record->wType == DNS_TYPE_A)
		{
			ip_list.push_back(dns_record->Data.A.IpAddress);
		}
		
		dns_record = dns_record->pNext;
	}

	//
	//	Free memory allocated for DNS records 
	//
	DnsRecordListFree(dns_record, DnsFreeRecordListDeep);
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
	char ipv4_buf[16 + 1] = { 0 };
	if (NULL == InetNtopA(AF_INET,
						  &ipv4,
						  ipv4_buf,
						  sizeof(ipv4_buf)))
	{
		log_err "InetNtopA() failed. wsa gle=%u",
			WSAGetLastError()
			log_end;
		return _null_stringa;
		
	}
	return std::string(ipv4_buf);
}

/// @brief	
std::wstring 
ipv4_to_strw(
	_In_ uint32_t ip_netbyte_order
)
{
	in_addr ia;
	ia.s_addr = ip_netbyte_order;
	return ipv4_to_strw(ia);
}

/// @brief	
std::wstring
ipv4_to_strw(
	_In_ in_addr& ipv4
)
{
	wchar_t ipv4_buf[16 + 1] = { 0 };
	if (NULL == InetNtopW(AF_INET,
						  &ipv4,
						  ipv4_buf,
						  sizeof(ipv4_buf)))
	{
		log_err "InetNtopW() failed. wsa gle=%u",
			WSAGetLastError()
			log_end;
		return _null_stringw;
	}
	return std::wstring(ipv4_buf);
}

/// @brief  
bool
str_to_ipv4(
	_In_ const char* const ipv4,
	_Out_ uint32_t& ip_netbyte_order
)
{
	_ASSERTE(NULL != ipv4);
	if (NULL != ipv4)
	{
		in_addr ipv4_addr = { 0 };
		int ret = InetPtonA(AF_INET,
							ipv4,
							&ipv4_addr);
		switch (ret)
		{
		case 1:
			ip_netbyte_order = ipv4_addr.s_addr;
			return true;    // success
		case 0:
			log_warn "invalid ipv4 string. input=%s",
				ipv4
				log_end;
			return false;
		case -1:
			log_err "InetPtonA() failed. input=%s, wsa gle=%u",
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
str_to_ipv4(
	_In_ const wchar_t* ipv4,
	_Out_ uint32_t& ip_netbyte_order
)
{
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
			log_warn "invalid ipv4 string. input = %ws",
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
uint32_t 
str_to_ipv4(
	_In_ const char* const ipv4
)
{
	uint32_t nb_ip = 0;
	if (!str_to_ipv4(ipv4, nb_ip))
	{
		return 0;
	}
	
	return nb_ip;
}

/// @brief	
uint32_t 
str_to_ipv4(
	_In_ const wchar_t* const ipv4
)
{
	uint32_t nb_ip = 0;
	if (!str_to_ipv4(ipv4, nb_ip))
	{
		return 0;
	}

	return nb_ip;
}



/// @brief  
std::string
ipv6_to_str(
	_In_ in_addr6& ipv6
)
{
	char ipv6_buf[46 + 1] = { 0 };
	if (NULL == InetNtopA(AF_INET6,
						  &ipv6,
						  ipv6_buf,
						  sizeof(ipv6_buf)))
	{
		log_err "InetNtopA() failed. wsa gle=%u",
			WSAGetLastError()
			log_end;
		return _null_stringa;
	}
	return std::string(ipv6_buf);
}

std::wstring 
ipv6_to_strw(
	_In_ in_addr6& ipv6
)
{
	wchar_t ipv6_buf[46 + 1] = { 0 };
	if (NULL == InetNtopW(AF_INET6,
						  &ipv6,
						  ipv6_buf,
						  sizeof(ipv6_buf)))
	{
		log_err "InetNtopW() failed. wsa gle=%u",
			WSAGetLastError()
			log_end;
		return _null_stringw;
		
	}
	return std::wstring(ipv6_buf);
}

/// @brief  
bool
str_to_ipv6(
	_In_ const char* const ipv6,
	_Out_ in_addr6& ip_netbyte_order
)
{
	_ASSERTE(NULL != ipv6);
	if (NULL != ipv6)
	{
		int ret = InetPtonA(AF_INET6, ipv6, &ip_netbyte_order);
		switch (ret)
		{
		case 1:
			return true;    // success
		case 0:
			log_warn "invalid ipv6 string. input = %ws",
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
str_to_ipv6(
	_In_ const wchar_t* ipv6,
	_Out_ in_addr6& ip_netbyte_order
)
{
	_ASSERTE(NULL != ipv6);
	if (NULL != ipv6)
	{
		int ret = InetPtonW(AF_INET6, ipv6, &ip_netbyte_order);
		switch (ret)
		{
		case 1:
			return true;    // success
		case 0:
			log_warn "invalid ipv6 string. input = %ws",
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
	_Out_ std::list<std::string>& ip_list
)
{
	std::list<PInetAdapter> adapters;
	if (true != get_inet_adapters(adapters))
	{
		log_err "get_inet_adapters() failed." log_end;
		return false;
	}

	for (const auto& adapter : adapters)
	{
		for (const auto& ip_info : adapter->ip_info_list)
		{
			ip_list.push_back(std::move(ipv4_to_str(ip_info->ip)));			
		}
		delete adapter;	//!!!
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
	_Out_ std::list<uint32_t>& broadcast_list
)
{
	std::list<PInetAdapter> adapters;
	if (true != get_inet_adapters(adapters))
	{
		log_err "get_inet_adapters() failed." log_end;
		return false;
	}

	for (const auto& adapter : adapters)
	{
		for (const auto& ip_info : adapter->ip_info_list)
		{
			broadcast_list.push_back(ip_info->ip | ~ip_info->mask);			
		}
		delete adapter;	//!!
	}
	adapters.clear();

	//
	//	255.255.255.255(0xffffffff) 추가
	//
	broadcast_list.push_back(0xffffffff);

	return true;
}

/// @brief	Localhost 의 대표(?) ip 를 리턴한다. 
std::string
get_representative_ip_v4(
)
{
	//
	//	1순위: 
	//		dns, gateway 가 설정된 어댑터의 첫번째 IP
	//	2순위:
	//		gateway 가 설정된 어댑터의 첫번째 IP
	//	3순위: 
	//		첫번째 IP
	// 
	std::list<PInetAdapter> adapters;
	if (true != get_inet_adapters(adapters))
	{
		log_err "get_inet_adapters() failed." log_end;
		return "127.0.0.1";
	}

	std::string ip;
	do
	{
		//
		//	1순위 찾기
		//
		for (const auto& adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			if (true != adapter->gateway_list.empty())
			{
				if (true != adapter->dns_list.empty())
				{
					ip = ipv4_to_str(adapter->ip_info_list[0]->ip);
					break;
				}
			}
		}
		if (true != ip.empty()) break;

		//
		//	2순위 찾기
		//
		for (const auto& adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			if (true != adapter->gateway_list.empty())
			{
				ip = ipv4_to_str(adapter->ip_info_list[0]->ip);
				break;
			}
		}
		if (true != ip.empty()) break;

		//
		//	3순위
		//
		for (const auto& adapter : adapters)
		{
			if (adapter->ip_info_list.empty()) continue;
			ip = ipv4_to_str(adapter->ip_info_list[0]->ip);
			break;
		}

	} while (false);

	//
	//	Free
	//	
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
	return (true != ip.empty()) ? ip : "127.0.0.1";
}

/// @brief  `ip_str` 주소를 가진 interface 의 mac 주소를 알아낸다. 
std::string
get_mac_by_ip_v4(
	_In_ const char* ip_str
)
{
	_ASSERTE(nullptr != ip_str);
	if (nullptr == ip_str) return false;

	uint32_t ip = 0xffffffff;
	if (true != str_to_ipv4(MbsToWcsEx(ip_str).c_str(), ip))
	{
		log_err "str_to_ipv4() failed. ip=%s",
			ip_str
			log_end;
		return "00-00-00-00-00-00";
	}
	_ASSERTE(0xffffffff != ip);
	if (0xffffffff == ip) return "00-00-00-00-00-00";

	//
	//	어댑터 정보를 가져온다.
	//
	std::list<PInetAdapter> adapters;
	if (true != get_inet_adapters(adapters))
	{
		log_err "get_inet_adapters() failed." log_end;
		return "00-00-00-00-00-00";
	}

	//
	//	IP 와 매칭한다.
	//
	bool matched = false;
	std::string mac;
	for (const auto& adapter : adapters)
	{
		if (true == adapter->ip_info_list.empty()) continue;
		for (const auto& ip_info : adapter->ip_info_list)
		{
			if (ip == ip_info->ip)
			{
				mac = mac_to_str(adapter->physical_address);
				matched = true;
				break;
			}
		}

		if (true == matched) break;
	}

	//
	//	Free
	//
	std::for_each(adapters.cbegin(), 
				  adapters.cend(), 
				  [](const PInetAdapter& p)
	{
		_ASSERTE(nullptr != p);
		delete p;
	});
	adapters.clear();

	return (true == mac.empty()) ? "00-00-00-00-00-00" : mac;
}

/// @brief	MAC 주소를 문자열로 변환한다.
std::string mac_to_str(_In_ const MacAddrType mac)
{
	char buf[(2 * 6) + 5 + 1] = { 0 };
	if (!SUCCEEDED(StringCbPrintfA(buf,
								   sizeof(buf),
								   "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
								   mac[0], mac[1], mac[2], 
								   mac[3], mac[4], mac[5])))
	{
		return std::string("N/A");
	}
	else
	{
		return std::string(buf);
	}
}

/// @brief	GetAddrInfoEx() wrapper 함수  
///			host_name 에는 아래의 입력형식을 지원함
///				- FQDN (e.g. www.google.com)
///				- 호스트 이름 (e.g. DESKTOP-IQPVKKP)
///			성공시 해당 호스트에 할당된 IPv4/IPv6 주소 목록을 리턴한다.
bool
get_addr_infow(
	_In_ const wchar_t* host_name,
	_Out_opt_ std::list<in_addr>* const ipv4_addrs,
	_Out_opt_ std::list<in6_addr>* const ipv6_addrs
)
{
	_ASSERTE(nullptr != host_name);
	if (nullptr == host_name) return false;

	if (nullptr == ipv4_addrs && nullptr == ipv6_addrs)
	{
		_ASSERTE(!"Oops!");
		log_err "No address list specified." log_end;
		return false;
	}

	PADDRINFOEXW result = nullptr;

	//
	// A value of AF_UNSPEC for ai_family indicates the caller will accept 
	//		only the AF_INET and AF_INET6 address families. Note that AF_UNSPEC 
	//		and PF_UNSPEC are the same.
	// A value of zero for ai_socktype indicates the caller will accept any socket type.
	// A value of zero for ai_protocol indicates the caller will accept any protocol.
	// The ai_addrlen member must be set to zero.
	// The ai_canonname member must be set to NULL.
	// The ai_addr member must be set to NULL.
	// The ai_next member must be set to NULL.	
	//
	ADDRINFOEXW hint = { 0 };
	hint.ai_family = AF_UNSPEC;

	int ret = GetAddrInfoExW(host_name,
							 nullptr,		// Service name
							 NS_ALL,
							 nullptr,
							 &hint,
							 &result,
							 nullptr,
							 nullptr,
							 nullptr,
							 nullptr);
	if (NO_ERROR != ret)
	{
		// Ref to error code
		// https://docs.microsoft.com/ko-kr/windows/win32/winsock/windows-sockets-error-codes-2
		// 
		log_err "GetAddrInfoExW() failed. name=%ws, wgle=%d",
			host_name,
			WSAGetLastError()
			log_end;
		return false;
	}

	//
	//	Save result to smart pointer
	//
	using info_ptr = std::unique_ptr<ADDRINFOEXW, void(*)(ADDRINFOEXW*)>;
	info_ptr result_guard(result, [](ADDRINFOEXW* p) 
	{
		FreeAddrInfoEx(p); 
	});

	for (PADDRINFOEXW ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		switch (ptr->ai_family) 
		{
		case AF_UNSPEC:			
			break;
		
		case AF_INET:
		{
			if (ipv4_addrs)
			{
				sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				ipv4_addrs->push_back(sockaddr_ipv4->sin_addr);				
			}
			break;
		}
		
		case AF_INET6:
		{
			if (ipv6_addrs)
			{
				sockaddr_in6* sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
				ipv6_addrs->push_back(sockaddr_ipv6->sin6_addr);
			}
			break;
		}		
		default:
			break;
		}		
	}

	return true;
}








/// @brief	예약된 IP 주소인 경우 true 를 리턴한다.
bool
is_reserved_ipv4(
	_In_ uint32_t ip_netbyte_order, 
	_In_ bool include_private	
)
{
	if (0 == ip_netbyte_order) return true;
	if (0xffffffff == ip_netbyte_order) return true;

	for (int i = 0; i < __crnam; ++i)
	{
		if (true != include_private && 
			true == __reserved_net_and_masks[i].is_private)
		{
			continue;
		}
		
		if (__reserved_net_and_masks[i].netbyte_order_ip == 
			(ip_netbyte_order & __reserved_net_and_masks[i].netbyte_order_mask))
		{
			return true;
		}
	}

	return false;
}
