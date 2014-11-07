/**----------------------------------------------------------------------------
 * _test_asm.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:10:30 23:26 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

#ifdef __cplusplus 
extern "C" {
#endif

ULONG64 sum64(ULONG64 a, ULONG64 b, ULONG64 c, ULONG64 d);
void trampoline();
ULONG64 direct_jump();
ULONG64 indirect_jump();
void push_mov_ret();
void push_mov_ret2();


#ifdef __cplusplus 
}
#endif



/**
 * @brief	
**/
bool test_asm_func()
{
	ULONG64 ret = sum64(1,1,1,1);
	if (4 != ret) return false;

	//trampoline();
	ret = direct_jump();
	ret = indirect_jump();
	push_mov_ret();
	push_mov_ret2();			// crash!


	ULONG64 addr = (ULONG64)test_asm_func;
	ULONG32 low  = addr & 0x00000000ffffffff;
	ULONG32 high = (addr & 0xffffffff00000000) >> 32;


	return true;
}