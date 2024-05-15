/**
 * @file    NTP client
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    05.08.2024 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>

#include "_MyLib/src/Win32Utils.h"
using boost::asio::ip::udp;

#define NTP_TIMESTAMP_DELTA         2208988800ull

// NTP ��Ŷ ����ü
#pragma pack(1)

struct ntp_packet {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_id;

    // 32 bits. Reference time-stamp seconds.
    uint32_t ref_timestamp_sec;
    // 32 bits. Reference time-stamp fraction of a second.
    uint32_t ref_timestamp_sec_frac;

    // 32 bits. Originate time-stamp seconds.
    uint32_t orig_timestamp_sec;
    // 32 bits. Originate time-stamp fraction of a second.
    uint32_t orig_timestamp_sec_frac;

    // 32 bits. Received time-stamp seconds.
    uint32_t received_timestamp_sec;
    // 32 bits. Received time-stamp fraction of a second.
    uint32_t received_timestamp_sec_frac;

    // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t transmited_timestamp_sec;
    // 32 bits. Transmit time-stamp fraction of a second.
    uint32_t transmited_timestamp_sec_frac;
};
#pragma pack()

//// ���н� Ÿ�ӽ������� ���� �ð�(FILETIME)���� ��ȯ
//FILETIME UnixTimeToFileTime(uint64_t unix_time) {
//    ULONGLONG ll = unix_time * 10000000 + 116444736000000000;
//    FILETIME ft;
//    ft.dwLowDateTime = (DWORD)ll;
//    ft.dwHighDateTime = ll >> 32;
//    return ft;
//}

// Ŭ�� ������ �� ����Ʈ�� ���� ���
std::pair<int64_t, int64_t> 
CalculateOffsetDelay(
    const ntp_packet& packet, 
    const std::chrono::time_point<std::chrono::system_clock>& recv_time
) 
{
    using namespace std::chrono;

    // refac
    //// ���� �ð� ��������
    //auto t1 = system_clock::now();

    //// ��Ŷ���� �ð� ����
    //uint64_t t2 = packet.recv_time;
    //uint64_t t3 = packet.transmit_time;

    //// ����Ʈ�� ���� ���
    //auto round_trip_delay = duration_cast<microseconds>(t1 - recv_time).count() / 2;

    //// Ŭ�� ������ ���
    //auto clock_offset = (t2 - t3 + recv_time.time_since_epoch().count() - t1.time_since_epoch().count()) / 2;

    //return std::make_pair(clock_offset, round_trip_delay);
    return std::make_pair(0, 0);
}


/// @brief  �ý����� �ð��� NTP ������ �ð��� ����(delta)���� �� ������ ����Ѵ�.
/// @remark NTP �� Ÿ�ӽ������� UNIX timestamp �� ����ϱ� ������ �� ������ ���� 
///         ��� �ϴ°��� �ǹ̾���.
bool ntp_client() 
{
    try 
    {
        boost::asio::io_context io_context;

        // NTP ������ �ּ� ����
        udp::resolver resolver(io_context);
        udp::resolver::results_type endpoints = resolver.resolve("time.windows.com", "123");

        // UDP ���� ����
        udp::socket socket(io_context);
        socket.open(udp::v4());

        // NTP ��Ŷ �ʱ�ȭ
        ntp_packet packet;
        memset(&packet, 0, sizeof(packet));
        packet.li_vn_mode = (0x3 << 6) | (4 << 3) | 3; // 0b11100011

        // ������ NTP ��Ŷ ����
        socket.send_to(boost::asio::buffer(&packet, sizeof(packet)), *endpoints.begin());

        // �����κ��� ���� �ޱ�
        boost::array<char, sizeof(ntp_packet)> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);

        // ���� �ð� ���
        FILETIME ft_recv = now_as_filetime();

        // �ð� ���� & ����Ʈ ���� ��ȯ
        auto* resp = reinterpret_cast<ntp_packet*>(recv_buffer.data());
        
        //  These two fields contain the time-stamp seconds as the packet left the NTP
        //  server. The number of seconds correspond to the seconds passed since 1900.
        //  ntohl() converts the bit/byte order from the network's to host's
        //  "endianness".
        resp->transmited_timestamp_sec = swap_endian_32(resp->transmited_timestamp_sec);
        //resp->transmited_timestamp_sec_frac = swap_endian_32(resp->transmited_timestamp_sec_frac);

        // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch)
        // from when the packet left the server. Subtract 70 years worth of seconds
        // from the seconds since 1900. This leaves the seconds since the UNIX epoch
        // of 1970.
        // (1900)---------(1970)**********(Time Packet Left the Server)
        uint32_t ntp_time = resp->transmited_timestamp_sec - NTP_TIMESTAMP_DELTA;

        //  resp->transmited_timestamp_sec  : t1, NTP ������ ��Ŷ�� ������ �ð�
        //  recv_time                       : t2, NTP ������ ���� �ð�
        // 
        //  t2 - t1 �� ���� �����־� �ð� ���� �����Ѵ�. (�ð����̴� �߻��� �� �ִ�)        
        //  
        //  1) ntp time �� unix timestamp �̹Ƿ� FILETIME ���� ��ȯ�Ѵ�.
        //  2) ntp time �� recv time �� ���� �и������� ������ ����Ѵ�. (������ ���� ���� �ִ�.)
        //  3) ntp time �� recv time �� ���� �ʷ� ��ȯ�ϰ�, �� ���� ���� ���� 0.5��(500 �и�������)
        //     �̻� ���̰� ���� ��� +1 �� �Ѵ�. (ntp time �� �ʴ��� ������, recv time �� FILETIME �̹Ƿ�
        //     �̷� ��Ȳ�� �߻��� ���� �ִ�)        
        FILETIME ft_ntp = unixtime_to_filetime(ntp_time);
        int64_t ms_delta = (int64_t)((int64_t)file_time_to_int(&ft_recv) - (int64_t)file_time_to_int(&ft_ntp)) / _file_time_to_msec;
        int64_t s_delta = ms_delta / 1000;          // ��
        if (ms_delta % 1000 >= 500)
        {
            s_delta += 1;
        }
            
        // test

        // ���
        std::cout   
            << "now      (System  ): " << time_now_to_str(false, true) 
            << std::endl;
        
        //std::cout
        //    << "NTP Time (        ): " << resp->transmited_timestamp_sec
        //    << "." << resp->transmited_timestamp_sec_frac
        //    << std::endl;

        FILETIME ft = unixtime_to_filetime(ntp_time);        
        std::cout << "NTP Time (UNIX    ): " << file_time_to_str(&ft, true, true) << std::endl;
        std::cout << "NTP Time (FILETIME): " << file_time_to_str(&ft, true, true) << std::endl;
        std::cout << "Delta    (        ): " << s_delta << " (sec)" << std::endl;


        //std::cout << "Clock Offset: " << offset_delay.first << " microseconds" << std::endl;
        //std::cout << "Round-Trip Delay: " << offset_delay.second << " microseconds" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return true;
}