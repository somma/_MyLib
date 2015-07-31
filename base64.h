/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

/// 
#include <string>

// [ by somma ]
// character set 에 따라서 결과가 달라질 수 있으므로 ascii 문자열이 아닌 경우 (한글, 특수문자가 포함된 문자열)
// bytes_to_encode 는 반드시 utf8 인코딩된 문자열을 사용해야 한다. 
// 
// #1) multibyte -> ucs16 -> utf8 -> base64 순서로...
// #2) ucs16 -> utf8 -> base64 
// 
// 디코딩할 때도 당연히 base64 decoded (utf8 로 간주) --> ucs16 으로 변경해서 사용

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);