/**
 * @file    Custom UNICODE_STRING substring routine test
 * @brief
 * @ref
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    12/28/2023 15:13
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength / 2), length_is((Length) / 2)] USHORT* Buffer;
#else // MIDL_PASS
	_Field_size_bytes_part_opt_(MaximumLength, Length) PWCH   Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;


/// @brief	
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
wchar_t*
uni_wcsstr(
	_In_ const PUNICODE_STRING haystack,
	_In_ const PUNICODE_STRING needle,
	_In_ bool reverse
)
{
	//PAGED_CODE();

	if (nullptr == haystack || nullptr == needle)
	{
		return nullptr;
	}

	if (haystack->Length == 0 || needle->Length == 0)
	{
		return nullptr;
	}

	if (needle->Length > haystack->Length)
	{
		return nullptr;
	}

	if (needle->Buffer[0] == L'\0')
	{
		return haystack->Buffer;
	}

	USHORT const cnt_haystack = haystack->Length / sizeof(wchar_t);
	USHORT const cnt_needle = needle->Length / sizeof(wchar_t);

	// needle 의 사이즈를 품을 수 있는 영역까지만 검사하면 되므로.
	USHORT const last_index = cnt_haystack - cnt_needle;

	if (true != reverse)
	{
		for (int i_hay = 0; i_hay <= last_index; ++i_hay)
		{
			USHORT i_needle = 0;
			for (; i_needle < cnt_needle; ++i_needle)
			{
				if ((wchar_t)haystack->Buffer[i_hay + i_needle] != (wchar_t)needle->Buffer[i_needle])
				{
					// move next character in haystack
					break;
				}
			}

			if (i_needle == cnt_needle)
			{
				return (wchar_t*)&haystack->Buffer[i_hay];
			}
		}
	}
	else
	{
		for (int i_hay = last_index; i_hay >= 0; --i_hay)
		{
			USHORT i_needle = 0;
			for (; i_needle < cnt_needle; ++i_needle)
			{
				if ((wchar_t)haystack->Buffer[i_hay+i_needle] != (wchar_t)needle->Buffer[i_needle])
				{
					// move next character in haystack
					break;
				}
			}

			if (i_needle == cnt_needle)
			{
				return (wchar_t*)&haystack->Buffer[i_hay];
			}
		}
	}

	return nullptr;
}


/// @brief 
/// @return 
bool test_uni_wcsstr()
{
	struct test_struct
	{
		const wchar_t* haystack;
		const wchar_t* needle;
		const bool expected;
	} 
	data[] = 
	{
		{L"012340123401234", L"", true},
		{L"012340123401234", L"123", true},
		{nullptr, nullptr, false}
	};


	for (int i = 0; i < sizeof(test_struct); ++i)
	{
		if (nullptr == data[i].haystack) break;

		UNICODE_STRING hay;
		hay.Buffer = (wchar_t*)data[i].haystack;
		// test 를 위해서 일부러 null 문자 길이를 뺌
		hay.Length = hay.MaximumLength = (USHORT)wcslen(data[i].haystack)*sizeof(wchar_t) - sizeof(wchar_t);
		
		UNICODE_STRING needle;
		needle.Buffer = (wchar_t*)data[i].needle;
		needle.Length = needle.MaximumLength = (USHORT)wcslen(data[i].needle) * sizeof(wchar_t) - sizeof(wchar_t);

		bool reverse = true;
		wchar_t* result = uni_wcsstr(&hay, &needle, reverse);
		log_info
			"hay=%ws, needle=%ws, reverse=%s, result=%ws",
			hay.Buffer,
			needle.Buffer,
			reverse ? "true" : "false",
			result
			log_end;

		reverse = false;
		result = uni_wcsstr(&hay, &needle, reverse);
		log_info
			"hay=%ws, needle=%ws, reverse=%s, result=%ws",
			hay.Buffer,
			needle.Buffer,
			reverse ? "true" : "false",
			result
			log_end;
	}
	
	return true;

}