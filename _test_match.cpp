/**
 * @file    _test_match.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    11/12/2023 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include "_MyLib/src/match.h"

bool test_match()
{
	_mem_check_begin
	{
		struct ss
		{
			const char* text;
			const char* pattern;
			bool expected;

		} ss_list [] = {
			{
				"https://www.naver.com",
				"htt?://*.naver.com",
				false
			},
			{
				"https://www.naver.com",
				"h*://?.naver.com",
				false
			},
			{
				"https://www.naver.com",
				"h*://*.naver.com",
				true
			},
			{
				"https://tivan.naver.com/sc2/12/",
				"http*://*.naver.com",
				false
			},
			{
				"https://tivan.naver.com/sc2/12/",
				"http?://*.naver.com/*",
				true
			},
			{
				"https://tivan.naver.com/sc2/12/",
				"http?://*.naver.com/*",
				true
			},
			{
				"https://www.naver.com",
				"http*://*.*.naver.com",
				false
			}
		};

		for (int i = 0; i < sizeof(ss_list) / sizeof(struct ss); ++i)
		{
			fprintf(stdout,
					"test=%s, match=%s, expected=%s, result=%s \n",
					ss_list[i].text, ss_list[i].pattern,
					ss_list[i].expected == true ? "O" : "X",
					true == match_string(ss_list[i].text, ss_list[i].pattern) ? "O" : "X");
					
		}
	}
	_mem_check_end;

	return true;
}