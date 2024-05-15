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

// NTP 패킷 구조체
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

//// 유닉스 타임스탬프를 파일 시간(FILETIME)으로 변환
//FILETIME UnixTimeToFileTime(uint64_t unix_time) {
//    ULONGLONG ll = unix_time * 10000000 + 116444736000000000;
//    FILETIME ft;
//    ft.dwLowDateTime = (DWORD)ll;
//    ft.dwHighDateTime = ll >> 32;
//    return ft;
//}

// 클록 오프셋 및 라운드트립 지연 계산
std::pair<int64_t, int64_t> 
CalculateOffsetDelay(
    const ntp_packet& packet, 
    const std::chrono::time_point<std::chrono::system_clock>& recv_time
) 
{
    using namespace std::chrono;

    // refac
    //// 현재 시간 가져오기
    //auto t1 = system_clock::now();

    //// 패킷에서 시간 추출
    //uint64_t t2 = packet.recv_time;
    //uint64_t t3 = packet.transmit_time;

    //// 라운드트립 지연 계산
    //auto round_trip_delay = duration_cast<microseconds>(t1 - recv_time).count() / 2;

    //// 클록 오프셋 계산
    //auto clock_offset = (t2 - t3 + recv_time.time_since_epoch().count() - t1.time_since_epoch().count()) / 2;

    //return std::make_pair(clock_offset, round_trip_delay);
    return std::make_pair(0, 0);
}


/// @brief  시스템의 시각과 NTP 서버의 시각의 차이(delta)값을 초 단위로 계산한다.
/// @remark NTP 의 타임스탬프는 UNIX timestamp 를 사용하기 때문에 초 이하의 값을 
///         계산 하는것은 의미없다.
bool ntp_client() 
{
    try 
    {
        boost::asio::io_context io_context;

        // NTP 서버의 주소 설정
        udp::resolver resolver(io_context);
        udp::resolver::results_type endpoints = resolver.resolve("time.windows.com", "123");

        // UDP 소켓 생성
        udp::socket socket(io_context);
        socket.open(udp::v4());

        // NTP 패킷 초기화
        ntp_packet packet;
        memset(&packet, 0, sizeof(packet));
        packet.li_vn_mode = (0x3 << 6) | (4 << 3) | 3; // 0b11100011

        // 서버로 NTP 패킷 전송
        socket.send_to(boost::asio::buffer(&packet, sizeof(packet)), *endpoints.begin());

        // 서버로부터 응답 받기
        boost::array<char, sizeof(ntp_packet)> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);

        // 현재 시간 기록
        FILETIME ft_recv = now_as_filetime();

        // 시간 추출 & 바이트 오더 변환
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

        //  resp->transmited_timestamp_sec  : t1, NTP 서버가 패킷을 전송한 시간
        //  recv_time                       : t2, NTP 응답을 받은 시간
        // 
        //  t2 - t1 한 값을 더해주어 시간 값을 보정한다. (시간차이는 발생할 수 있다)        
        //  
        //  1) ntp time 은 unix timestamp 이므로 FILETIME 으로 변환한다.
        //  2) ntp time 과 recv time 의 차를 밀리세컨드 단위로 계산한다. (음수가 나올 수도 있다.)
        //  3) ntp time 과 recv time 의 차를 초로 변환하고, 초 단위 이하 값이 0.5초(500 밀리세컨드)
        //     이상 차이가 나는 경우 +1 초 한다. (ntp time 은 초단위 이지만, recv time 은 FILETIME 이므로
        //     이런 상황이 발생할 수도 있다)        
        FILETIME ft_ntp = unixtime_to_filetime(ntp_time);
        int64_t ms_delta = (int64_t)((int64_t)file_time_to_int(&ft_recv) - (int64_t)file_time_to_int(&ft_ntp)) / _file_time_to_msec;
        int64_t s_delta = ms_delta / 1000;          // 초
        if (ms_delta % 1000 >= 500)
        {
            s_delta += 1;
        }
            
        // test

        // 출력
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