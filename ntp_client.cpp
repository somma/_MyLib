/**
 * @file    NTP client
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    05.08.2024 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include "_MyLib/src/ntp_client.h"

/// @brief  NTP �������� �ð� ���� �� ������ �����Ѵ�.
int64_t 
get_ntp_time_delta(
    _In_ const std::string& server, 
    _In_ const int timeout_sec
)
{
    try
    {
        boost::asio::io_context io_context;
        NTPClient client(io_context, server, timeout_sec);
        client.get_ntp_time();
        io_context.run();
        //std::cout
        //    << "Local  : " << time_now_to_str(true, true) << std::endl
        //    << "NTP    : " << file_time_to_str(client.get_ntp_as_filetime(), true, true) << " " << std::endl
        //    << "delta  : " << client.get_delta_sec() << (" sec") << std::endl;
        return client.get_delta_sec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        
        //  ������ �߻��� ��� �ý��� ���� Ÿ���� �״�� ����ؾ� �ϹǷ�
        //  0 �� �����Ѵ�.
        return 0ll;
    }
}

