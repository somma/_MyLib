/**
 * @file    _test_cppjson.cpp
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2020/07/20 21:51 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "json/json.h"

bool test_cpp_joson()
{
	_mem_check_begin
	{
		Json::Value root;

		FILETIME now; GetSystemTimeAsFileTime(&now);

		//
		//	json 쓰기
		//

		root["group_guid"] = "group_guid";
		root["host_guid"] = "host_guid";
		root["process_guid"] = "process_guid";
		root["process_timestamp"] = file_time_to_int(&now);
		root["event_id"] = 1500;
		root["event_block_key"] = 991827;
		root["event_timestamp"] = file_time_to_int(&now);
		root["pid"] = 8892;
		root["flag"] = 0xff;

		root["parent"]["process_guid"] = "process guid";
		root["parent"]["process_timestamp"] = file_time_to_int(&now);

		//
		//	array 쓰기
		//
		Json::Value ar;
		for (int i = 0; i < 8; ++i)
		{
			ar.append(i);
		}
		root["array"] = ar;

		//
		//	Json::Value --> string
		//
		std::stringstream strm;
		Json::StreamWriterBuilder f;
		try
		{
			std::unique_ptr<Json::StreamWriter> writer(f.newStreamWriter());
			writer->write(root, &strm);
			log_info
				"%s",
				strm.str().c_str()
				log_end;
		}
		catch (const std::exception& e)
		{
			log_err
				"exception, event id=%s, e=%s",
				root["event_id"].asString().c_str(), 
				e.what()
				log_end;
		}


		//
		//	Change Json style
		//
		f.settings_["commentStyle"] = "None";
		f.settings_["indentation"] = "";

		{
			clear_sstream(strm);
			try
			{
				std::unique_ptr<Json::StreamWriter> writer(f.newStreamWriter());
				writer->write(root, &strm);
				log_info
					"%s",
					strm.str().c_str()
					log_end;
			}
			catch (const std::exception& e)
			{
				log_err
					"exception, event id=%s, e=%s",
					root["event_id"].asString().c_str(),
					e.what()
					log_end;
			}
		} 
	}
	_mem_check_end;
	return true;
}