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


/// @brief  �ý����� �ð��� NTP ������ �ð��� ����(delta)���� �� ������ ����Ѵ�.
/// @remark NTP �� Ÿ�ӽ������� UNIX timestamp �� ����ϱ� ������ �� ������ ���� 
///         ��� �ϴ°��� �ǹ̾���.
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

    /// @brief  NTP ������ �ð����� ��û�ϰ� ���ýð����� ���̰��� ������Ʈ�Ѵ�. 
    /// @return ������ �߻��ϰų� ��û�� ������ ��� ���ܸ� �߻���Ų��.
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

    /// @brief  NTP �������� �ð� ���� �� ������ �����Ѵ�.
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

    FILETIME _ntp_time;             // NTP ���� �ð�
    int64_t _delta_sec;             // NTP ������ Localtime �� ���� (�� ����)

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
            // NTP ��Ŷ �ʱ�ȭ
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

            // ���� �ð� ���
            FILETIME ft_recv = now_as_filetime();

            // �ð� ���� & ����Ʈ ���� ��ȯ
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
            _ntp_time = unixtime_to_filetime(ntp_time);
            const int64_t ms_delta = (int64_t)((int64_t)file_time_to_int(&ft_recv) - (int64_t)file_time_to_int(&_ntp_time)) / _file_time_to_msec;

            _delta_sec = ms_delta / 1000;          // ��
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

/// @brief  NTP �������� �ð� ���� �� ������ �����Ѵ�.
int64_t 
get_ntp_time_delta(
    _In_ const std::string& server,
    _In_ const int timeout_sec
);