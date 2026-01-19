/**
 * @file    NTP client
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    05.08.2024 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include <iostream>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <chrono>

#include "_MyLib/src/Win32Utils.h"
#include "_MyLib/src/log.h"

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


/// @brief  시스템의 시각과 NTP 서버의 시각의 차이(delta)값을 초 단위로 계산한다.
/// @remark NTP 의 타임스탬프는 UNIX timestamp 를 사용하기 때문에 초 이하의 값을 
///         계산 하는것은 의미없다.
class NTPClient {
public:
    NTPClient(
        boost::asio::io_context& io_context,
        const std::string& server,
        const int timeout_sec)
        :
        _socket(io_context),
        _resolver(io_context),
        _server(server),
        _timeout(io_context),
        _timeout_sec(timeout_sec),
        _ntp_time{ 0ul, 0ul },
        _delta_sec(0ll)
    {
    }

    /// @brief  NTP 서버에 시간값을 요청하고 로컬시간과의 차이값을 업데이트한다. 
    /// @return 에러가 발생하거나 요청이 실패한 경우 예외를 발생시킨다.
    void get_ntp_time()
    {
        boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), _server, "ntp");
        _resolver.async_resolve(query,
                                boost::bind(&NTPClient::on_resolve, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::results));
        _timeout.expires_from_now(boost::posix_time::seconds(_timeout_sec));
        _timeout.async_wait(boost::bind(&NTPClient::on_timeout, this,
                                        boost::asio::placeholders::error));
    }

    PFILETIME get_ntp_as_filetime()
    {
        return &_ntp_time;
    }

    /// @brief  NTP 서버와의 시간 차를 초 단위로 리턴한다.
    int64_t get_delta_sec()
    {
        return _delta_sec;
    }

private:
    boost::asio::ip::udp::socket _socket;
    boost::asio::ip::udp::resolver _resolver;
    std::string _server;
    boost::asio::deadline_timer _timeout;
    uint32_t _timeout_sec;
    std::array<uint8_t, 48> _recv_buf;

    FILETIME _ntp_time;             // NTP 서버 시각
    int64_t _delta_sec;             // NTP 서버와 Localtime 의 차이 (초 단위)

private:
    void on_resolve(
        _In_ const boost::system::error_code& err,
        _In_ boost::asio::ip::udp::resolver::results_type endpoints
    )
    {
        if (!err)
        {
            _socket.async_connect(*endpoints.begin(),
                                  boost::bind(&NTPClient::on_connect, this,
                                              boost::asio::placeholders::error));
        }
        else
        {
            log_err
                "can not resolve the name. server=%s, err=%s",
                _server.c_str(),
                err.message().c_str()
                log_end;
        }
    }

    void on_connect(
        _In_ const boost::system::error_code& err
    )
    {
        if (!err)
        {
            // NTP 패킷 초기화
            ntp_packet packet;
            memset(&packet, 0, sizeof(packet));
            packet.li_vn_mode = (0x3 << 6) | (4 << 3) | 3; // 0b11100011
            _socket.async_send(boost::asio::buffer(&packet, sizeof(packet)),
                               boost::bind(&NTPClient::on_send,
                                           this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            log_err
                "can not connect. server=%s, err=%s",
                _server.c_str(),
                err.message().c_str()
                log_end;
        }
    }

    void on_send(
        _In_ const boost::system::error_code& err,
        _In_ std::size_t /*bytes_transferred*/)
    {
        if (!err)
        {
            _socket.async_receive(boost::asio::buffer(_recv_buf),
                                  boost::bind(&NTPClient::on_receive, this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            log_err
                "can not send. server=%s, err=%s",
                _server.c_str(),
                err.message().c_str()
                log_end;
        }
    }

    void on_receive(
        _In_ const boost::system::error_code& err,
        _In_ std::size_t bytes_transferred)
    {
        if (!err && bytes_transferred == _recv_buf.size())
        {
            _timeout.cancel();

            // 현재 시간 기록
            FILETIME ft_recv = now_as_filetime();

            // 시간 추출 & 바이트 오더 변환
            auto* resp = reinterpret_cast<ntp_packet*>(_recv_buf.data());

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
            const uint32_t ntp_time = resp->transmited_timestamp_sec - NTP_TIMESTAMP_DELTA;

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
            _ntp_time = unixtime_to_filetime(ntp_time);
            const int64_t ms_delta = (int64_t)((int64_t)file_time_to_int(&ft_recv) - (int64_t)file_time_to_int(&_ntp_time)) / _file_time_to_msec;

            _delta_sec = ms_delta / 1000;          // 초
            if (ms_delta % 1000 >= 500)
            {
                _delta_sec += 1;
            }
        }
        else
        {
            log_err
                "can not receive. server=%s, err=%s",
                _server.c_str(),
                err.message().c_str()
                log_end;
        }
    }

    void on_timeout(_In_ const boost::system::error_code& err)
    {
        if (!err)
        {
            _socket.cancel();
            log_err
                "request time out. server=%s, err=%s",
                _server.c_str(),
                err.message().c_str()
                log_end;
        }
    }
};

/// @brief  NTP 서버와의 시간 차를 초 단위로 리턴한다.
int64_t 
get_ntp_time_delta(
    _In_ const std::string& server,
    _In_ const int timeout_sec
);