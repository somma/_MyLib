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

bool ntp_client()
{
    _mem_check_begin
    {
        try
        {
            boost::asio::io_context io_context;
            NTPClient client(io_context, "pool.ntp.org", 1);
            client.get_ntp_time();
            io_context.run();

            std::cout
                << "Local  : " << time_now_to_str(true, true) << std::endl
                << "NTP    : " << file_time_to_str(client.get_ntp_as_filetime(), true, true) << " " << std::endl
                << "delta  : " << client.get_delta_sec() << (" sec") << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
    _mem_check_end;    

    return true;
}