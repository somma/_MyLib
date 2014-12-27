/**----------------------------------------------------------------------------
 * _test_x64.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:11:5 15:56 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"



/**
 * @brief	

__int64 func_a(int a, float b, int c, int d, int e)
{
000000013FAF0990 44 89 4C 24 20       mov         dword ptr [rsp+20h],r9d  
000000013FAF0995 44 89 44 24 18       mov         dword ptr [rsp+18h],r8d  
000000013FAF099A F3 0F 11 4C 24 10    movss       dword ptr [rsp+10h],xmm1  
000000013FAF09A0 89 4C 24 08          mov         dword ptr [rsp+8],ecx  
000000013FAF09A4 57                   push        rdi  
	return a+c+e;	
000000013FAF09A5 8B 44 24 20          mov         eax,dword ptr [c]  
000000013FAF09A9 8B 4C 24 10          mov         ecx,dword ptr [a]  
000000013FAF09AD 03 C8                add         ecx,eax  
000000013FAF09AF 8B C1                mov         eax,ecx  
000000013FAF09B1 03 44 24 30          add         eax,dword ptr [e]  
000000013FAF09B5 48 98                cdqe  
}
000000013FAF09B7 5F                   pop         rdi  
000000013FAF09B8 C3                   ret  

**/
__int64 func_a(int a, double b, int c, int d, int e)
{
	return a+c+e;	
}



/**
 * @brief	

 rec func_b(int a, double b, int c, int d, int e)
{
000000013F3DBB10 44 89 4C 24 20       mov         dword ptr [rsp+20h],r9d  
000000013F3DBB15 F2 0F 11 54 24 18    movsd       mmword ptr [rsp+18h],xmm2  
000000013F3DBB1B 89 54 24 10          mov         dword ptr [rsp+10h],edx  
000000013F3DBB1F 48 89 4C 24 08       mov         qword ptr [rsp+8],rcx  
000000013F3DBB24 56                   push        rsi  
000000013F3DBB25 57                   push        rdi  
000000013F3DBB26 48 83 EC 68          sub         rsp,68h  
000000013F3DBB2A 48 8B FC             mov         rdi,rsp  
000000013F3DBB2D B9 1A 00 00 00       mov         ecx,1Ah  
000000013F3DBB32 B8 CC CC CC CC       mov         eax,0CCCCCCCCh  
000000013F3DBB37 F3 AB                rep stos    dword ptr [rdi]  
000000013F3DBB39 48 8B 8C 24 80 00 00 00 mov         rcx,qword ptr [rsp+80h]  
000000013F3DBB41 48 8B 05 A0 A4 0E 00 mov         rax,qword ptr [__security_cookie (13F4C5FE8h)]  
000000013F3DBB48 48 33 C4             xor         rax,rsp  
000000013F3DBB4B 48 89 44 24 50       mov         qword ptr [rsp+50h],rax  
	rec r = {0};
000000013F3DBB50 C7 44 24 28 00 00 00 00 mov         dword ptr [rsp+28h],0  
000000013F3DBB58 48 8D 44 24 2C       lea         rax,[rsp+2Ch]  
000000013F3DBB5D 48 8B F8             mov         rdi,rax  
000000013F3DBB60 33 C0                xor         eax,eax  
000000013F3DBB62 B9 08 00 00 00       mov         ecx,8  
000000013F3DBB67 F3 AA                rep stos    byte ptr [rdi]  
	return r;
000000013F3DBB69 48 8D 44 24 28       lea         rax,[rsp+28h]  
000000013F3DBB6E 48 8B BC 24 80 00 00 00 mov         rdi,qword ptr [rsp+80h]  
000000013F3DBB76 48 8B F0             mov         rsi,rax  
000000013F3DBB79 B9 0C 00 00 00       mov         ecx,0Ch  
000000013F3DBB7E F3 A4                rep movs    byte ptr [rdi],byte ptr [rsi]  
000000013F3DBB80 48 8B 84 24 80 00 00 00 mov         rax,qword ptr [rsp+80h]  
}
000000013F3DBB88 48 8B F8             mov         rdi,rax  
000000013F3DBB8B 48 8B CC             mov         rcx,rsp  
000000013F3DBB8E 48 8D 15 2B 49 0A 00 lea         rdx,[13F4804C0h]  
000000013F3DBB95 E8 06 6B 08 00       call        _RTC_CheckStackVars (13F4626A0h)  
000000013F3DBB9A 48 8B C7             mov         rax,rdi  
000000013F3DBB9D 48 8B 4C 24 50       mov         rcx,qword ptr [rsp+50h]  
000000013F3DBBA2 48 33 CC             xor         rcx,rsp  
000000013F3DBBA5 E8 D6 6A 08 00       call        __security_check_cookie (13F462680h)  
000000013F3DBBAA 48 83 C4 68          add         rsp,68h  
000000013F3DBBAE 5F                   pop         rdi  
000000013F3DBBAF 5E                   pop         rsi  
000000013F3DBBB0 C3                   ret  
**/
typedef struct _rec
{
	int a;
	int b;
	int c;
} rec;

rec func_b(int a, double b, int c, int d, int e)
{
	rec r = {0};
	return r;
}

/**
 * @brief	

bool test_x64_calling_convension()
{
000000013FAF09C0 40 57                push        rdi  
000000013FAF09C2 48 83 EC 40          sub         rsp,40h  
000000013FAF09C6 48 8B FC             mov         rdi,rsp  
000000013FAF09C9 B9 10 00 00 00       mov         ecx,10h  
000000013FAF09CE B8 CC CC CC CC       mov         eax,0CCCCCCCCh  
000000013FAF09D3 F3 AB                rep stos    dword ptr [rdi]  
	__int64 ret = func_a(10, 0.1, 20, 30, 40);
000000013FAF09D5 C7 44 24 20 28 00 00 00 mov         dword ptr [rsp+20h],28h
000000013FAF09DD 41 B9 1E 00 00 00    mov         r9d,1Eh  
000000013FAF09E3 41 B8 14 00 00 00    mov         r8d,14h  
000000013FAF09E9 F3 0F 10 0D F3 36 04 00 movss       xmm1,dword ptr [__real@3dcccccd (13FB340E4h)]  
000000013FAF09F1 B9 0A 00 00 00       mov         ecx,0Ah  
000000013FAF09F6 E8 9C 0F F7 FF       call        func_a (13FA61997h)  
000000013FAF09FB 48 89 44 24 30       mov         qword ptr [ret],rax  


	return true;
000000013FAF0A00 B0 01                mov         al,1  
}
000000013FAF0A02 48 83 C4 40          add         rsp,40h  
000000013FAF0A06 5F                   pop         rdi  
000000013FAF0A07 C3                   ret  

**/
bool test_x64_calling_convension()
{
	__int64 ra = func_a(10, 0.1, 20, 30, 40);

	/*
000000013F3DBC16 C7 44 24 28 05 00 00 00 mov         dword ptr [rsp+28h],5  
000000013F3DBC1E C7 44 24 20 04 00 00 00 mov         dword ptr [rsp+20h],4  
000000013F3DBC26 41 B9 03 00 00 00		 mov         r9d,3  
000000013F3DBC2C F2 0F 10 15 74 EF 0B 00 movsd       xmm2,mmword ptr [__real@3ff0000000000000 (13F49ABA8h)]  
000000013F3DBC34 BA 01 00 00 00          mov         edx,1  
000000013F3DBC39 48 8D 8C 24 80 00 00 00 lea         rcx,[rsp+80h]  ; <<
000000013F3DBC41 E8 90 87 FF FF       call        func_b (13F3D43D6h)  
	*/
	rec rb = func_b(1, 1.0, 3, 4, 5);


	return true;
}


