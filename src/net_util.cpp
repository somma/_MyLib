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


/// @brief	Winsock �� �ʱ�ȭ�Ѵ�.
///			
///			!!����!!
///			WSAStartup() / WSACleanup() �� ȣ��Ƚ���� ��Ȯ�� ��ġ�ؾ� �Ѵ�. 
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

/// @brief	Winsock �� �����Ѵ�. 
///			
///			!!����!!
///			WSAStartup() / WSACleanup() �� ȣ��Ƚ���� ��Ȯ�� ��ġ�ؾ� �Ѵ�. 
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
get_net_adapters(
	_In_ ULONG net_family,
	_Out_ std::vector<PNetAdapter>& adapters
)
{
	if ((net_family != AF_INET) && (net_family != AF_INET6))
	{
		log_err
			"Invalid net family. Only AF_INET(2), AF_INET(23) supported."
			log_end;
		return false;
	}

	//
	//	flag �� 0 �϶� unicast, anycast, multicast ������ �����´�. 
	//	���⿡ include, exclude flag �� �����ؼ� ����� �� �ְ� �Ѱ� ������, 
	//	include, exclude �� ���̸� ���� ������ ��� ����� �ȳ���
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
	PNetAdapter adapter = nullptr;
	ULONG subnet_mask = 0;
	PIP_ADAPTER_UNICAST_ADDRESS unicast_addr = nullptr;
	PIP_ADAPTER_DNS_SERVER_ADDRESS dns = nullptr;
	PIP_ADAPTER_GATEWAY_ADDRESS gateway = nullptr;

	std::string str;
	PIP_ADAPTER_ADDRESSES cur = address;
	while (nullptr != cur)
	{
		///	IF_TYPE_SOFTWARE_LOOPBACK �������̽� ������ ����
		if (cur->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
		{
			goto _next;
		}

		///	���� Ȱ��ȭ �Ǿ����� �ʴ� ����� ������ �������� �ʴ´�. 
		if (IfOperStatusUp != cur->OperStatus)
		{
			goto _next;
		}

		//
		//	NetAdapter ��ü�� �����ϰ�, ����Ʈ�� �߰��Ѵ�. 
		//
		adapter = new NetAdapter();
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
		//	data link layer �������̽��� �ƴ� ��� 0 �� �� �ִ�. 
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
		unicast_addr = cur->FirstUnicastAddress;
		while (nullptr != unicast_addr)
		{
			if (true == SocketAddressToStr(&unicast_addr->Address, str))
			{
				//
				//	subnet mask
				//	PrefixLength�� �̿��ؼ� subnet mask�� ���Ѵ�.
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
		dns = cur->FirstDnsServerAddress;
		while (nullptr != dns)
		{
			if (true == SocketAddressToStr(&dns->Address, str))
			{
				adapter->dns_list.push_back(str);
			}
			dns = dns->Next;
		}

		///	Gateway
		gateway = cur->FirstGatewayAddress;
		while (nullptr != gateway)
		{
			if (true == SocketAddressToStr(&gateway->Address, str))
			{
				adapter->gateway_list.push_back(str);
			}
			gateway = gateway->Next;
		}

		//
		//	����� ��ü �߰�
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
///	@remark	WSAAddressToStringA �Լ��� deprecated �Ǿ��� ������ W ���� �Լ��� ����ϰ�, 
///			address string ���ڿ��� ��ȯ�Ѵ�.
bool
SocketAddressToStr(
	_In_ const SOCKADDR* addr,
	_Out_ std::string& addr_str
)
{
	_ASSERTE(nullptr != addr);
	if (nullptr == addr) return false;

	//
	//	�ʿ��� ����� ���
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
	//	���� �Ҵ�
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
	//	��ȯ�õ�
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
	//	wchar -> char �� ��ȯ�ϰ�, ����
	//
	addr_str = WcsToMbsEx(buf.get());
	return true;
}

///	@brief	
/*
	�������� ip -> DNS ��ȸ �׽�Ʈ �غ���

	> nslookup
	> set type = PTR
	> set debug

	> 103.27.148.71
	Server:  google - public - dns - a.google.com
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
		responsible mail addr = read - txt - record - of - zone - first - dns - admin.apnic.net
		serial = 36246
		refresh = 7200 (2 hours)
		retry = 1800 (30 mins)
		expire = 604800 (7 days)
		default TTL = 172800 (2 days)

	------------
		*** google - public - dns - a.google.com can't find 71.148.27.103.in-addr.arpa.: Non-existent domain



	> 103.243.220.231
	Server:  google - public - dns - a.google.com
	Address : 8.8.8.8

	------------
	Got answer :
	HEADER:
		opcode = QUERY, id = 6, rcode = NOERROR
		header flags : response, want recursion, recursion avail.
		questions = 1, answers = 1, authority records = 0, additional = 0

		QUESTIONS :
			231.220.243.103.in - addr.arpa, type = PTR, class = IN
			ANSWERS :
		->  231.220.243.103.in - addr.arpa
		name = 175.bm - nginx - loadbalancer.mgmt.sin1.adnexus.net
		ttl = 3004 (50 mins 4 secs)

	------------
	Non - authoritative answer :
	231.220.243.103.in - addr.arpa
		name = 175.bm - nginx - loadbalancer.mgmt.sin1.adnexus.net
		ttl = 3004 (50 mins 4 secs)

*/
bool
ip_to_dns(
	_In_ uint32_t ip_netbyte_order,
	_In_ bool cache_only,
	_Out_ std::wstring& domain_name
)
{
	std::string dns_query_ip;
	PDNS_RECORD dns_record = nullptr;

	//
	//	127.0.0.1 -> 1.0.0.127.IN-ADDR.ARPA
	//
	dns_query_ip = ipv4_to_str(swap_endian_32(ip_netbyte_order)).append(".IN-ADDR.ARPA");

	//--------------------------------------------------------------------------------
	//
	// DNS_QUERY_STANDARD �ɼ��� ����� ��� 
	//	- local cache -> query with UDP -> query with TCP (���䵥���Ͱ� �߸����)
	//	- DNS_QUERY_NO_MULTICAST : DNS �������� ��û�� ����(��?)
	//	- DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE �ɼ��� �ָ� tcp �� ���� ��õ��� ���� ��
	//
	//--------------------------------------------------------------------------------
	//
	//	DNS_QUERY_STANDARD vs DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE ���� ��
	//	ipconfig /flushdns ������� cache �� ���� 188 ���� ip �� dns �� ��ȯ�׽�Ʈ
	//	
	//	DNS_QUERY_STANDARD �ɼ� ���
	//		1st try) total=188, succ=70, cache=0,  wire=70, elapsed= 145830.484375 ms
	//		2nd try) total=188, succ=70, cache=57, wire=13, elapsed= 137857.656250 ms
	//	1st try �� 2nd try �� ���̰� ũ�� �ʴ�(10��). DNS �̸��� ���� IP ���� ��κ��� 
	//	�ð��� ��ƸԱ� ����
	// 
	//	DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE �ɼ� ���
	//		1st try) total=188, succ=70, cache=0, wire=70, elapsed=   33709.382813 ms
	//		2nd try) total=188, succ=70, cache=69, wire=1, elapsed=   14285.799805 ms
	//	��ü������ DNS_QUERY_STANDARD ���� �� 5�� ������. DNS �̸� ���� IP ��ȸ �ð��� ���������
	//	������ ������ 1st try, 2nd try �� ���̰� DNS_QUERY_STANDARD ���� �� ŭ
	//
	//--------------------------------------------------------------------------------
	DNS_STATUS status = DnsQuery_W(MbsToWcsEx(dns_query_ip.c_str()).c_str(),
								   DNS_TYPE_PTR,
								   (true == cache_only) ? DNS_QUERY_NO_WIRE_QUERY : (DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE),
								   NULL,
								   &dns_record,
								   NULL);
	if (ERROR_SUCCESS != status)
	{
		if (DNS_ERROR_RECORD_DOES_NOT_EXIST == status || DNS_ERROR_RCODE_NAME_ERROR == status)
		{
			//
			//	dns �̸� ���� ���, ... �� �׷��Ŵ�... ����.
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

		domain_name = L"";
		return false;
	}
	_ASSERTE(nullptr != dns_record);
	if (nullptr == dns_record)
	{
		domain_name = L"";
		return false;
	}
	domain_name = dns_record->Data.PTR.pNameHost;

	//
	//	Free memory allocated for DNS records 
	//
	DnsRecordListFree(dns_record, DnsFreeRecordListDeep);
	return true;
}

/// @brief	www.google.com -> 1.1.1.1 �� ��ȯ�ϴ� �Լ�
bool
dns_to_ip(
	_In_ const wchar_t* domain_name,
	_In_ bool cache_only,
	_Out_ std::vector<uint32_t>& ip_list
)
{
	std::string dns_query_ip;
	PDNS_RECORD dns_record = nullptr;

	DNS_STATUS status = DnsQuery_W(domain_name, 
								   DNS_TYPE_A,
								   (true == cache_only) ? DNS_QUERY_NO_WIRE_QUERY : (DNS_QUERY_NO_MULTICAST | DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE),
								   NULL,
								   &dns_record,
								   NULL);
	if (ERROR_SUCCESS != status)
	{
		if (DNS_ERROR_RECORD_DOES_NOT_EXIST == status || DNS_ERROR_RCODE_NAME_ERROR == status)
		{
			//
			//	��ȿ���� ���� ������ ����
			//
			log_dbg "DnsQuery(cache_only=%s) failed. domain=%ws, status=%u",
				true == cache_only ? "O" : "X",
				domain_name,
				status
				log_end;
		}
		else
		{
			log_dbg "DnsQuery(cache_only=%s) failed. ip=%s, status=%u",
				true == cache_only ? "O" : "X",
				dns_query_ip.c_str(),
				status
				log_end;
		}

		return false;
	}
	_ASSERTE(nullptr != dns_record);
	if (nullptr == dns_record)
	{
		log_err "DnsQuery(cache_only=%s) succeeded but no recored. domain=%ws",
			true == cache_only ? "O" : "X",
			domain_name
			log_end;

		return false;
	}
	
	while(dns_record != nullptr)
	{
		ip_list.push_back(dns_record->Data.A.IpAddress);
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
	_Out_ std::vector<std::string>& ip_list
)
{
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
		}
		delete adapter;
	}
	adapters.clear();

	//
	//	255.255.255.255(0xffffffff) �߰�
	//
	broadcast_list.push_back(0xffffffff);

	return true;
}

/// @brief	Localhost �� ��ǥ(?) ip �� �����Ѵ�. 
std::string
get_representative_ip_v4(
)
{
	//
	//	1����: 
	//		dns, gateway �� ������ ������� ù��° IP
	//	2����:
	//		gateway �� ������ ������� ù��° IP
	//	3����: 
	//		ù��° IP
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
		//	1���� ã��
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
		//	2���� ã��
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
		//	3����
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
	std::for_each(adapters.begin(), adapters.end(), [](_In_ PNetAdapter p)
	{
		_ASSERTE(nullptr != p);
		delete p;
	});
	adapters.clear();
	return (true != ip.empty()) ? ip : "127.0.0.1";
}

/// @brief  `ip_str` �ּҸ� ���� interface �� mac �ּҸ� �˾Ƴ���. 
std::string
get_mac_by_ip_v4(
	_In_ const char* ip_str
)
{
	_ASSERTE(NULL != ip_str);
	if (NULL == ip_str) return false;

	//
	//	����� ������ �����´�.
	//
	std::vector<PNetAdapter> adapters;
	if (true != get_net_adapters(AF_INET, adapters))
	{
		log_err "get_net_adapters() failed." log_end;
		return "00:00:00:00:00:00";
	}

	//
	//	IP �� ��Ī�Ѵ�.
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

/// @brief	MAC �ּҸ� ���ڿ��� ��ȯ�Ѵ�.
std::string mac_to_str(_In_ const MacAddrType mac)
{
	char buf[(2 * 6) + 5 + 1] = { 0 };
	if (!SUCCEEDED(StringCbPrintfA(buf,
								   sizeof(buf),
								   "%.2X%.2X%.2X%.2X%.2X%.2X",
								   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
	)))
	{
		return std::string("N/A");
	}
	else
	{
		return std::string(buf);
	}
}

/// @brief	GetAddrInfoEx() wrapper �Լ�  
///			host_name ���� �Ʒ��� �Է������� ������
///				- FQDN (e.g. www.google.com)
///				- ȣ��Ʈ �̸� (e.g. DESKTOP-IQPVKKP)
///			������ �ش� ȣ��Ʈ�� �Ҵ�� IPv4/IPv6 �ּ� ����� �����Ѵ�.
bool
get_addr_infow(
	_In_ const wchar_t* host_name,
	_Out_opt_ std::vector<in_addr>* const ipv4_addrs,
	_Out_opt_ std::vector<in6_addr>* const ipv6_addrs
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







