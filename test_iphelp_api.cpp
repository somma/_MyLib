
#include "stdafx.h"
#include "_MyLib/src/net_util.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iphlpapi.h>

#include <stdio.h>
#include <time.h>

// Need to link with Iphlpapi.lib and Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
/* Note: could also use malloc() and free() */



int exec_iphelp_api_sample()
{
    /* Some general variables */
    ULONG ulOutBufLen;
    DWORD dwRetVal;
    unsigned int i;
  
    /* variables used for GetNetworkParams */
    FIXED_INFO *pFixedInfo;
    IP_ADDR_STRING *pIPAddr;

    /* variables used for GetAdapterInfo */
    IP_ADAPTER_INFO *pAdapterInfo;
    IP_ADAPTER_INFO *pAdapter;

    /* variables used to print DHCP time info */
    struct tm newtime;
    char buffer[32];
    errno_t error;

    /* variables used for GetInterfaceInfo */
    IP_INTERFACE_INFO *pInterfaceInfo;

    /* variables used for GetIpAddrTable */
    MIB_IPADDRTABLE *pIPAddrTable;
    DWORD dwSize;
    IN_ADDR IPAddr;
    //char *strIPAddr;
	char addr[16] = { 0 };
	std::string ip_str;

    /* variables used for AddIpAddress */
//    UINT iaIPAddress;
//    UINT imIPMask;
//    ULONG NTEContext;
//    ULONG NTEInstance;

    /* variables used for GetIpStatistics */
    MIB_IPSTATS *pStats;

    /* variables used for GetTcpStatistics */
    MIB_TCPSTATS *pTCPStats;

    printf("------------------------\n");
    printf("This is GetNetworkParams\n");
    printf("------------------------\n");

    pFixedInfo = (FIXED_INFO *) MALLOC(sizeof (FIXED_INFO));
    if (pFixedInfo == NULL) {
        printf("Error allocating memory needed to call GetNetworkParams\n");
        return 1;
    }
    ulOutBufLen = sizeof (FIXED_INFO);

    if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pFixedInfo);
        pFixedInfo = (FIXED_INFO *) MALLOC(ulOutBufLen);
        if (pFixedInfo == NULL) {
            printf("Error allocating memory needed to call GetNetworkParams\n");
            return 1;
        }
    }

	dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen);
    if (dwRetVal != NO_ERROR) 
	{
        printf("GetNetworkParams failed with error %d\n", dwRetVal);
        if (pFixedInfo)
            FREE(pFixedInfo);
        return 1;
    } else {
        printf("\tHost Name: %s\n", pFixedInfo->HostName);
        printf("\tDomain Name: %s\n", pFixedInfo->DomainName);
        printf("\tDNS Servers:\n");
        printf("\t\t%s\n", pFixedInfo->DnsServerList.IpAddress.String);

        pIPAddr = pFixedInfo->DnsServerList.Next;
        while (pIPAddr) {
            printf("\t\t%s\n", pIPAddr->IpAddress.String);
            pIPAddr = pIPAddr->Next;
        }

        printf("\tNode Type: ");
        switch (pFixedInfo->NodeType) {
        case 1:
            printf("%s\n", "Broadcast");
            break;
        case 2:
            printf("%s\n", "Peer to peer");
            break;
        case 4:
            printf("%s\n", "Mixed");
            break;
        case 8:
            printf("%s\n", "Hybrid");
            break;
        default:
            printf("\n");
        }

        printf("\tNetBIOS Scope ID: %s\n", pFixedInfo->ScopeId);

        if (pFixedInfo->EnableRouting)
            printf("\tIP Routing Enabled: Yes\n");
        else
            printf("\tIP Routing Enabled: No\n");

        if (pFixedInfo->EnableProxy)
            printf("\tWINS Proxy Enabled: Yes\n");
        else
            printf("\tWINS Proxy Enabled: No\n");

        if (pFixedInfo->EnableDns)
            printf("\tNetBIOS Resolution Uses DNS: Yes\n");
        else
            printf("\tNetBIOS Resolution Uses DNS: No\n");
    }

    /* Free allocated memory no longer needed */
    if (pFixedInfo) {
        FREE(pFixedInfo);
        pFixedInfo = NULL;
    }

    printf("------------------------\n");
    printf("This is GetAdaptersInfo\n");
    printf("------------------------\n");

    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdapterInfo\n");
        return 1;
    }
    ulOutBufLen = sizeof (IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdapterInfo\n");
            return 1;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != NO_ERROR) {
        printf("GetAdaptersInfo failed with error %d\n", dwRetVal);
        if (pAdapterInfo)
            FREE(pAdapterInfo);
        return 1;
    }

    pAdapter = pAdapterInfo;
    while (pAdapter) {
        printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
        printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
        printf("\tAdapter Addr: \t");
        for (i = 0; i < (int) pAdapter->AddressLength; i++) {
            if (i == (pAdapter->AddressLength - 1))
                printf("%.2X\n", (int) pAdapter->Address[i]);
            else
                printf("%.2X-", (int) pAdapter->Address[i]);
        }
        printf("\tIP Address: \t%s\n",
               pAdapter->IpAddressList.IpAddress.String);
        printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);

        printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
        printf("\t***\n");

        if (pAdapter->DhcpEnabled) {
            printf("\tDHCP Enabled: \tYes\n");
            printf("\tDHCP Server: \t%s\n",
                   pAdapter->DhcpServer.IpAddress.String);

            printf("\tLease Obtained: ");
            /* Display local time */
            error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseObtained);
            if (error)
                printf("\tInvalid Argument to _localtime32_s\n");

            else {
                // Convert to an ASCII representation 
                error = asctime_s(buffer, 32, &newtime);
                if (error)
                    printf("Invalid Argument to asctime_s\n");
                else
                    /* asctime_s returns the string terminated by \n\0 */
                    printf("%s", buffer);
            }

            printf("\tLease Expires:  ");
            error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseExpires);
            if (error)
                printf("Invalid Argument to _localtime32_s\n");
            else {
                // Convert to an ASCII representation 
                error = asctime_s(buffer, 32, &newtime);
                if (error)
                    printf("Invalid Argument to asctime_s\n");
                else
                    /* asctime_s returns the string terminated by \n\0 */
                    printf("%s", buffer);
            }
        } else
            printf("\tDHCP Enabled: \tNo\n");

        if (pAdapter->HaveWins) {
            printf("\tHave Wins: \tYes\n");
            printf("\tPrimary Wins Server: \t%s\n",
                   pAdapter->PrimaryWinsServer.IpAddress.String);
            printf("\tSecondary Wins Server: \t%s\n",
                   pAdapter->SecondaryWinsServer.IpAddress.String);
        } else
            printf("\tHave Wins: \tNo\n");

        printf("\n");
        pAdapter = pAdapter->Next;
    }

    printf("------------------------\n");
    printf("This is GetInterfaceInfo\n");
    printf("------------------------\n");

    pInterfaceInfo = (IP_INTERFACE_INFO *) MALLOC(sizeof (IP_INTERFACE_INFO));
    if (pInterfaceInfo == NULL) {
        printf("Error allocating memory needed to call GetInterfaceInfo\n");
        return 1;
    }
    ulOutBufLen = sizeof (IP_INTERFACE_INFO);
    if (GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen) ==
        ERROR_INSUFFICIENT_BUFFER) {
        FREE(pInterfaceInfo);
        pInterfaceInfo = (IP_INTERFACE_INFO *) MALLOC(ulOutBufLen);
        if (pInterfaceInfo == NULL) {
            printf("Error allocating memory needed to call GetInterfaceInfo\n");
            return 1;
        }
        printf("\t The size needed for the output buffer ulLen = %ld\n",
               ulOutBufLen);
    }

    if ((dwRetVal = GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen)) == NO_ERROR) {
        printf("\tNum Adapters: %ld\n\n", pInterfaceInfo->NumAdapters);
        for (i = 0; i < (unsigned int) pInterfaceInfo->NumAdapters; i++) {
            printf("\tAdapter Index[%d]: %ld\n", i,
                   pInterfaceInfo->Adapter[i].Index);
            printf("\tAdapter Name[%d]:  %ws\n\n", i,
                   pInterfaceInfo->Adapter[i].Name);
        }
        printf("GetInterfaceInfo call succeeded.\n");
    } else {
        LPVOID lpMsgBuf = NULL;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                          (LPTSTR) & lpMsgBuf, 0, NULL)) {
            printf("\tError: %s", (char*)lpMsgBuf);
        }
        LocalFree(lpMsgBuf);
    }

    ///* If DHCP enabled, release and renew the IP address */
    ///* THIS WORKS BUT IT TAKES A LONG TIME AND INTERRUPTS NET CONNECTIONS */
    //if (pAdapterInfo->DhcpEnabled && pInterfaceInfo->NumAdapters) {
    //    printf("Calling IpReleaseAddress for Adapter[%d]\n", 0);
    //    if ((dwRetVal =
    //         IpReleaseAddress(&pInterfaceInfo->Adapter[0])) == NO_ERROR) {
    //        printf("Ip Release succeeded.\n");
    //    }
    //    if ((dwRetVal =
    //         IpRenewAddress(&pInterfaceInfo->Adapter[0])) == NO_ERROR) {
    //        printf("Ip Renew succeeded.\n");
    //    }
    //}

    /* Free allocated memory no longer needed */
    if (pAdapterInfo) {
        FREE(pAdapterInfo);
        pAdapterInfo = NULL;
    }
    if (pInterfaceInfo) {
        FREE(pInterfaceInfo);
        pInterfaceInfo = NULL;
    }

    printf("----------------------\n");
    printf("This is GetIpAddrTable\n");
    printf("----------------------\n");

    pIPAddrTable = (MIB_IPADDRTABLE *) MALLOC(sizeof (MIB_IPADDRTABLE));
    if (pIPAddrTable == NULL) {
        printf("Error allocating memory needed to call GetIpAddrTable\n");
        return 1;
    }
    dwSize = 0;
    IPAddr.S_un.S_addr = ntohl(pIPAddrTable->table[1].dwAddr);	
	ip_str = InetNtopA(AF_INET, (PVOID)&IPAddr, addr, 16);

    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIPAddrTable);
        pIPAddrTable = (MIB_IPADDRTABLE *) MALLOC(dwSize);
        if (pIPAddrTable == NULL) {
            printf("Error allocating memory needed to call GetIpAddrTable\n");
            return 1;
        }
    }

    if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) {
        printf("GetIpAddrTable failed with error %d\n", dwRetVal);
        if (pIPAddrTable)
            FREE(pIPAddrTable);
        return 1;
    }

    printf("\tNum Entries: %ld\n", pIPAddrTable->dwNumEntries);
    for (i = 0; i < (unsigned int) pIPAddrTable->dwNumEntries; i++) {
        printf("\n\tInterface Index[%d]:\t%ld\n", i,
               pIPAddrTable->table[i].dwIndex);
        IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwAddr;
		printf("\tIP Address[%d]:     \t%s\n", i, InetNtopA(AF_INET, (PVOID)&IPAddr, addr, 16));
        IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwMask;
		printf("\tSubnet Mask[%d]:    \t%s\n", i, InetNtopA(AF_INET, (PVOID)&IPAddr, addr, 16));
        IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwBCastAddr;
		printf("\tBroadCast[%d]:      \t%s (%ld)\n", i, InetNtopA(AF_INET, (PVOID)&IPAddr, addr, 16), pIPAddrTable->table[i].dwBCastAddr);
        printf("\tReassembly size[%d]:\t%ld\n", i, pIPAddrTable->table[i].dwReasmSize);
        printf("\tAddress Index[%d]:  \t%ld\n", i, pIPAddrTable->table[i].dwIndex);
        printf("\tType and State[%d]:", i);
        if (pIPAddrTable->table[i].wType & MIB_IPADDR_PRIMARY)
            printf("\tPrimary IP Address");
        if (pIPAddrTable->table[i].wType & MIB_IPADDR_DYNAMIC)
            printf("\tDynamic IP Address");
        if (pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)
            printf("\tAddress is on disconnected interface");
        if (pIPAddrTable->table[i].wType & MIB_IPADDR_DELETED)
            printf("\tAddress is being deleted");
        if (pIPAddrTable->table[i].wType & MIB_IPADDR_TRANSIENT)
            printf("\tTransient address");
        printf("\n");
    }

    //iaIPAddress = inet_addr("192.168.0.27");
    //imIPMask = inet_addr("255.255.255.0");

    //NTEContext = 0;
    //NTEInstance = 0;

    //if ((dwRetVal = AddIPAddress(iaIPAddress,
    //                             imIPMask,
    //                             pIPAddrTable->table[0].
    //                             dwIndex,
    //                             &NTEContext, &NTEInstance)) != NO_ERROR) {

    //    LPVOID lpMsgBuf;
    //    printf("\tError adding IP address.\n");

    //    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
    //                      (LPTSTR) & lpMsgBuf, 0, NULL)) {
    //        printf("\tError: %s", (char*)lpMsgBuf);
    //    }
    //    LocalFree(lpMsgBuf);
    //}

    //if ((dwRetVal = DeleteIPAddress(NTEContext)) != NO_ERROR) {
    //    printf("DeleteIPAddress failed with error %d\n", dwRetVal);
    //}

    /* Free allocated memory no longer needed */
    if (pIPAddrTable) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }

    printf("-------------------------\n");
    printf("This is GetIPStatistics()\n");
    printf("-------------------------\n");

    pStats = (MIB_IPSTATS *) MALLOC(sizeof (MIB_IPSTATS));
    if (pStats == NULL) {
        printf("Error allocating memory needed to call GetIpStatistics\n");
        return 1;
    }

    if ((dwRetVal = GetIpStatistics(pStats)) != NO_ERROR) {
        printf("GetIPStatistics failed with error %d\n", dwRetVal);
        if (pStats)
            FREE(pStats);
        return 1;
    }

    printf("\tNumber of IP addresses: %ld\n", pStats->dwNumAddr);
    printf("\tNumber of Interfaces: %ld\n", pStats->dwNumIf);
    printf("\tReceives: %ld\n", pStats->dwInReceives);
    printf("\tOut Requests: %ld\n", pStats->dwOutRequests);
    printf("\tRoutes: %ld\n", pStats->dwNumRoutes);
    printf("\tTimeout Time: %ld\n", pStats->dwReasmTimeout);
    printf("\tIn Delivers: %ld\n", pStats->dwInDelivers);
    printf("\tIn Discards: %ld\n", pStats->dwInDiscards);
    printf("\tTotal In: %ld\n", pStats->dwInDelivers + pStats->dwInDiscards);
    printf("\tIn Header Errors: %ld\n", pStats->dwInHdrErrors);

    /* Free allocated memory no longer needed */
    if (pStats) {
        FREE(pStats);
        pStats = NULL;
    }

    printf("-------------------------\n");
    printf("This is GetTCPStatistics()\n");
    printf("-------------------------\n");

    pTCPStats = (MIB_TCPSTATS *) MALLOC(sizeof (MIB_TCPSTATS));
    if (pTCPStats == NULL) {
        printf("Error allocating memory needed to call GetTcpStatistics\n");
        return 1;
    }

    if ((dwRetVal = GetTcpStatistics(pTCPStats)) != NO_ERROR) {
        printf("GetTcpStatistics failed with error %d\n", dwRetVal);
        if (pTCPStats)
            FREE(pTCPStats);
        return 1;
    }

    printf("\tActive Opens: %ld\n", pTCPStats->dwActiveOpens);
    printf("\tPassive Opens: %ld\n", pTCPStats->dwPassiveOpens);
    printf("\tSegments Recv: %ld\n", pTCPStats->dwInSegs);
    printf("\tSegments Xmit: %ld\n", pTCPStats->dwOutSegs);
    printf("\tTotal # Conxs: %ld\n", pTCPStats->dwNumConns);

    /* Free allocated memory no longer needed */
    if (pTCPStats) {
        FREE(pTCPStats);
        pTCPStats = NULL;
    }

    return 0;
}


/// https://msdn.microsoft.com/en-us/library/windows/desktop/aa366058(v=vs.85).aspx
int exec_iphelp_api_sample2(ULONG family)
{
 /* Declare and initialize variables */

//    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i = 0;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    // ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    IP_ADAPTER_PREFIX *pPrefix = NULL;

    //if (argc != 2) {
    //    printf(" Usage: getadapteraddresses family\n");
    //    printf("        getadapteraddresses 4 (for IPv4)\n");
    //    printf("        getadapteraddresses 6 (for IPv6)\n");
    //    printf("        getadapteraddresses A (for both IPv4 and IPv6)\n");
    //    exit(1);
    //}

    //if (atoi(argv[1]) == 4)
    //    family = AF_INET;
    //else if (atoi(argv[1]) == 6)
    //    family = AF_INET6;
	

    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the 
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen) == 
		ERROR_BUFFER_OVERFLOW) {
        FREE(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
    }

    if (pAddresses == NULL) {
        printf("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
        exit(1);
    }
    // Make a second call to GetAdapters Addresses to get the
    // actual data we want
    printf("Memory allocated for GetAdapterAddresses = %d bytes\n", outBufLen);
    printf("Calling GetAdaptersAddresses function with family = ");
    if (family == AF_INET)
        printf("AF_INET\n");
    if (family == AF_INET6)
        printf("AF_INET6\n");
    if (family == AF_UNSPEC)
        printf("AF_UNSPEC\n\n");

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            printf("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n",pCurrAddresses->Length);
            printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
			printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);

            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++)
                    pUnicast = pUnicast->Next;
                printf("\tNumber of Unicast Addresses: %d\n", i);
            } else
                printf("\tNo Unicast Addresses\n");

            pAnycast = pCurrAddresses->FirstAnycastAddress;
            if (pAnycast) {
                for (i = 0; pAnycast != NULL; i++)
                    pAnycast = pAnycast->Next;
                printf("\tNumber of Anycast Addresses: %d\n", i);
            } else
                printf("\tNo Anycast Addresses\n");

            pMulticast = pCurrAddresses->FirstMulticastAddress;
            if (pMulticast) {
                for (i = 0; pMulticast != NULL; i++)
                    pMulticast = pMulticast->Next;
                printf("\tNumber of Multicast Addresses: %d\n", i);
            } else
                printf("\tNo Multicast Addresses\n");

            pDnServer = pCurrAddresses->FirstDnsServerAddress;
            if (pDnServer) {
				for (i = 0; pDnServer != NULL; i++)
				{
					std::string addr_str;
					if (true == SocketAddressToStr(&pDnServer->Address, addr_str))
					{
						printf("DNS[%u]: %s",
							   i,
							   addr_str.c_str());
					}
					pDnServer = pDnServer->Next;
				}
                printf("\tNumber of DNS Server Addresses: %d\n", i);
            } else
                printf("\tNo DNS Server Addresses\n");

            printf("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
            printf("\tDescription: %wS\n", pCurrAddresses->Description);
            printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);

            if (pCurrAddresses->PhysicalAddressLength != 0) {
                printf("\tPhysical address: ");
                for (i = 0; i < pCurrAddresses->PhysicalAddressLength;
                     i++) {
                    if (i == (pCurrAddresses->PhysicalAddressLength - 1))
                        printf("%.2X\n",
                               (int) pCurrAddresses->PhysicalAddress[i]);
                    else
                        printf("%.2X-",
                               (int) pCurrAddresses->PhysicalAddress[i]);
                }
            }
            printf("\tFlags: %ld\n", pCurrAddresses->Flags);
            printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
            printf("\tIfType: %ld\n", pCurrAddresses->IfType);
            printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
            printf("\tIpv6IfIndex (IPv6 interface): %u\n",
                   pCurrAddresses->Ipv6IfIndex);
            printf("\tZoneIndices (hex): ");
            for (i = 0; i < 16; i++)
                printf("%lx ", pCurrAddresses->ZoneIndices[i]);
            printf("\n");

            pPrefix = pCurrAddresses->FirstPrefix;
            if (pPrefix) {
                for (i = 0; pPrefix != NULL; i++)
                    pPrefix = pPrefix->Next;
                printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
            } else
                printf("\tNo IP Adapter Prefix entries\n");

            printf("\n");

            pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n",
               dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
                              (LPTSTR) & lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", (char*)lpMsgBuf);
                LocalFree(lpMsgBuf);
                FREE(pAddresses);
                exit(1);
            }
        }
    }
    FREE(pAddresses);
    return 0;
}


bool test_iphelp_api()
{
	_CrtMemState memoryState = { 0 };
	_CrtMemCheckpoint(&memoryState);

	{
		//if (0 != exec_iphelp_api_sample()) return false;
		//if (0 != exec_iphelp_api_sample2(AF_INET)) return false;
		//if (0 != exec_iphelp_api_sample2(AF_INET6)) return false;
	}
	
	_CrtMemDumpAllObjectsSince(&memoryState);
	return true;
}