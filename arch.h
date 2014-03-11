/**************************************************************************
 * arch.h
 *-------------------------------------------------------------------------
 * 
 *-------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-------------------------------------------------------------------------
 * history:
 * 	2013-15-1 	created
**************************************************************************/
#ifndef _arch_header_
#define _arch_header_

#ifdef __cplusplus
extern "C" {
#endif 

#include <ntifs.h>
#include <ntintsafe.h>

#ifdef __cplusplus
}
#endif 

//warning C4201: nonstandard extension used : nameless struct/union
//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)


/* 
	_x86_ 이 붙은 자료구조는 x86/x64 의 사이즈가 다름
	_x64_ 포팅할 때 참고
*/


#pragma pack(1)


//> EFlags Register
typedef struct _X86_EFLAGS
{
	union
	{
		DWORD value;            // packed value
		
		struct
		{
			unsigned CF				:  1; //  0, Carry Flag
			unsigned defalt_1		:  1; //  1, 1
			unsigned PF				:  1; //  2, Parity Flag
			unsigned default_3		:  1; //  3, 0
			unsigned AF				:  1; //  4, Auxiliary Carry Flag
			unsigned default_5		:  1; //  5, 0
			unsigned ZF				:  1; //  6, Zero Flag
			unsigned SF				:  1; //  7, Sign Flag  << 여기 까지가 Status Flag

			unsigned TF				:  1; //  8, Trap Flag
			unsigned IF				:  1; //  9, Interrupt enable Flag
			unsigned DF				:  1; // 10, Direction Flag
			unsigned OF				:  1; // 11, Overflow Flag
			unsigned IOPL			:  2; // 12-13, I/O Privilege Level
			unsigned NT				:  1; // 14, Nested Task
			unsigned Default4		:  1; // 15, 0

			unsigned RF				:  1; // 16, Resume Flag
			unsigned VM				:  1; // 17, Virtual 8086 Mode
			unsigned AC				:  1; // 18, Alignment Check
			unsigned VIF			:  1; // 19, Virtual Interrupt Flag
			unsigned VIP			:  1; // 20, Virtual Interrupt Pending
			unsigned ID				:  1; // 21, ID Flag
			unsigned reserved_22	: 10; // 22
			
		};
	};
} X86_EFLAGS, *PX86_EFLAGS;

typedef struct _X64_RFLAGS
{
	union
	{
		UINT64 value;            // packed value
		
		struct
		{
			UINT64 CF				:  1; //  0, Carry Flag
			UINT64 defalt_1			:  1; //  1, 1
			UINT64 PF				:  1; //  2, Parity Flag
			UINT64 default_3		:  1; //  3, 0
			UINT64 AF				:  1; //  4, Auxiliary Carry Flag
			UINT64 default_5		:  1; //  5, 0
			UINT64 ZF				:  1; //  6, Zero Flag
			UINT64 SF				:  1; //  7, Sign Flag  << 여기 까지가 Status Flag

			UINT64 TF				:  1; //  8, Trap Flag
			UINT64 IF				:  1; //  9, Interrupt enable Flag
			UINT64 DF				:  1; // 10, Direction Flag
			UINT64 OF				:  1; // 11, Overflow Flag
			UINT64 IOPL				:  2; // 12, I/O Privilege Level
			UINT64 NT				:  1; // 13, Nested Task
			UINT64 Default4			:  1; // 15, 0

			UINT64 RF				:  1; // 16, Resume Flag
			UINT64 VM				:  1; // 17, Virtual 8086 Mode
			UINT64 AC				:  1; // 18, Alignment Check
			UINT64 VIF				:  1; // 19, Virtual Interrupt Flag
			UINT64 VIP				:  1; // 20, Virtual Interrupt Pending
			UINT64 ID				:  1; // 21, ID Flag
			UINT64 reserved_22		: 10; // 22
			UINT64 reserved_32		: 32; // 32-63
		};
	};
} X64_RFLAGS, *PX64_RFLAGS;

//> cr0 레지스터 
//	contains system control flags that control operating mode and state of the processor
typedef struct _X86_CR0
{
	union
	{
		UINT32 value;

		struct 
		{
			UINT32	PE				: 1;	// 0, protection enable
			UINT32	MP				: 1;	// 1, monitor coprocessor
			UINT32	EM				: 1;	// 2, emulation
			UINT32	TS				: 1;	// 3, task switched
			UINT32	ET				: 1;	// 4, extension type
			UINT32	NE				: 1;	// 5, numeric error
			UINT32	reserved_6_15	: 10;
			UINT32	WP				: 1;	// 16, write protect
			UINT32	reserved_17		: 1;
			UINT32	AM				: 1;	// 18, alignment mask
			UINT32	reserved_19_28	: 10;
			UINT32	NW				: 1;	// 29, not write-through
			UINT32	CD				: 1;	// 30, cache disable
			UINT32	PG				: 1;	// 31, paging, 1 == paging enabled
		} ;
	};

} X86_CR0, *PX86_CR0;

typedef struct _X64_CR0
{
	union
	{
		UINT64 value;

		struct 
		{
			UINT64	PE				: 1;	// 0, protection enable
			UINT64	MP				: 1;	// 1, monitor coprocessor
			UINT64	EM				: 1;	// 2, emulation
			UINT64	TS				: 1;	// 3, tqask switched
			UINT64	ET				: 1;	// 4, extension type
			UINT64	NE				: 1;	// 5, numeric error
			UINT64	reserved_6_15	: 10;
			UINT64	WP				: 1;	// 16, write protect
			UINT64	reserved_17		: 1;
			UINT64	AM				: 1;	// 18, alignment mask
			UINT64	reserved_19_28	: 10;
			UINT64	NW				: 1;	// 29, not write-through
			UINT64	CD				: 1;	// 30, cache diable
			UINT64	PG				: 1;	// 31, paging, 1 == paging enabled
			UINT64  reserved_32_63	: 32;	// reserved in x64
		};
	};

} X64_CR0, *PX64_CR0;

//> Intel CPU flags in CR0
#define X86_CR0_PE              0x00000001      /* Enable Protected Mode    (RW) */
#define X86_CR0_MP              0x00000002      /* Monitor Coprocessor      (RW) */
#define X86_CR0_EM              0x00000004      /* Require FPU Emulation    (RO) */
#define X86_CR0_TS              0x00000008      /* Task Switched            (RW) */
#define X86_CR0_ET              0x00000010      /* Extension type           (RO) */
#define X86_CR0_NE              0x00000020      /* Numeric Error Reporting  (RW) */
#define X86_CR0_WP              0x00010000      /* Supervisor Write Protect (RW) */
#define X86_CR0_AM              0x00040000      /* Alignment Checking       (RW) */
#define X86_CR0_NW              0x20000000      /* Not Write-Through        (RW) */
#define X86_CR0_CD              0x40000000      /* Cache Disable            (RW) */
#define X86_CR0_PG              0x80000000      /* Paging                   (RW) */


//> cr3 레지스터 (Page-Directory Base Register, PDBR)
typedef struct _X86_CR3 
{
	union
	{
		struct 
		{
    		unsigned value;
		};

    	struct
		{
			unsigned Reserved1 :  3;
			unsigned PWT       :  1; // page-level write-through
			unsigned PCD       :  1; // page-level cache disabled
			unsigned Reserved2 :  7;
			unsigned PFN       : 20; // page-frame number
		} ;
	};
} X86_CR3;

typedef struct _X64_CR3 
{
	union
	{
		struct 
		{
    		UINT64 value;
		};

    	struct
		{
			UINT64 Reserved1 :  3;
			UINT64 PWT       :  1; // page-level write-through
			UINT64 PCD       :  1; // page-level cache disabled
			UINT64 Reserved2 :  7;
			UINT64 PFN       : 52; // page-frame number
		} ;
	};
} X64_CR3;

//> cr4 레지스터 
//	contains a group of flags that enable several architectural extenstions 
//	and indicate operating system or executive support for specific processor capabilities.
typedef struct _X86_CR4
{
	union
	{
		unsigned value;

		struct 
		{
			UINT32 vme				: 1;		// 0, virtual 8086 mode extension
			UINT32 pvi				: 1;		// 1, protected-mode virtual interrupts
			UINT32 tsd				: 1;		// 2, time stamp disable
			UINT32 de				: 1;		// 3, debugging extension
			UINT32 pse				: 1;		// 4, page size extensions (enable 4Mb pages with 32bit paging)
			UINT32 pae				: 1;		// 5, physical address extension
			UINT32 mce				: 1;		
			UINT32 pge				: 1;
			UINT32 pce				: 1;
			UINT32 osfxsr			: 1;
			UINT32 osxmmexcpt		: 1;
			UINT32 reserved_11_12	: 2;
			UINT32 vmxe				: 1;		// 13, VMX-Enable bit (Virtual Machine Extensions)
			UINT32 smxe				: 1;		// 14, SMX-Enable bit (Safer Mode Extensions)
			UINT32 reserved_15_16	: 2;
			UINT32 pcide			: 1;
			UINT32 osxsave			: 1;
			UINT32 reserved_19_31	: 12;		//  reserved. set to zero				

		} ;
		
	};
} X86_CR4, *PX86_CR4;

typedef struct _X64_CR4
{
	union
	{
		UINT64 value;

		struct 
		{
			UINT64 vme				: 1;		// 0, virtual 8086 mode extension
			UINT64 pvi				: 1;		// 1, protected-mode virtual interrupts
			UINT64 tsd				: 1;		// 2, time stamp disable
			UINT64 de				: 1;		// 3, debugging extension
			UINT64 pse				: 1;		// 4, page size extensions (enable 4Mb pages with 32bit paging)
			UINT64 pae				: 1;		// 5, physical address extension
			UINT64 mce				: 1;		
			UINT64 pge				: 1;
			UINT64 pce				: 1;
			UINT64 osfxsr			: 1;
			UINT64 osxmmexcpt		: 1;
			UINT64 reserved_11_12	: 2;
			UINT64 vmxe				: 1;		// 13, VMX-Enable bit (Virtual Machine Extensions)
			UINT64 smxe				: 1;		// 14, SMX-Enable bit (Safer Mode Extensions)
			UINT64 reserved_15_16	: 2;
			UINT64 pcide			: 1;
			UINT64 osxsave			: 1;
			UINT64 reserved_19_64	: 44;	//  reserved. set to zero				

		} ;
	};
} X64_CR4, *PX64_CR4;

//> Intel CPU features in CR4
#define X86_CR4_VME			0x0001  /* enable vm86 extensions */
#define X86_CR4_PVI			0x0002  /* virtual interrupts flag enable */
#define X86_CR4_TSD			0x0004  /* disable time stamp at ipl 3 */
#define X86_CR4_DE			0x0008  /* enable debugging extensions */
#define X86_CR4_PSE			0x0010  /* enable page size extensions */
#define X86_CR4_PAE			0x0020  /* enable physical address extensions */
#define X86_CR4_MCE			0x0040  /* Machine check enable */
#define X86_CR4_PGE			0x0080  /* enable global pages */
#define X86_CR4_PCE			0x0100  /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR		0x0200  /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT	0x0400  /* enable unmasked SSE exceptions */
#define X86_CR4_VMXE		0x2000  /* enable VMX */ 


#ifdef _M_AMD64
	typedef X64_RFLAGS	FLAGS;
	typedef X64_CR0		CR0;
	typedef X64_CR3		CR3;
	typedef X64_CR4		CR4;
#elif defined(_M_IX86)
	typedef X86_EFLAGS	FLAGS;
	typedef X86_CR0		CR0;
	typedef X86_CR3		CR3;
	typedef X86_CR4		CR4;
#else
	#error !need to write code for this architecture
#endif


//> GDTR register 
typedef struct _X86_GDTR
{
	UINT16 limit;
	UINT32 linear_base;

} X86_GDTR;

typedef struct _X64_GDTR
{
	UINT16 limit;
	UINT64 linear_base;

} X64_GDTR;

//> IDTR register
typedef struct _X86_IDTR
{
	UINT16 limit;
	UINT32 linear_base;
	
}X86_IDTR;

typedef struct _X64_IDTR
{
	UINT16 limit;
	UINT64 linear_base;
	
}X64_IDTR;

#ifdef _M_AMD64
	typedef X64_GDTR	GDTR;
	typedef X64_IDTR	IDTR;
#elif defined(_M_IX86)
	typedef X86_GDTR	GDTR;
	typedef X86_IDTR	IDTR;
#else
	#error !need to write code for this architecture
#endif

//> segment selector (x86, x64 사이즈 동일)
//> vol3. 3.4.2
typedef struct _SEGMENT_SELECTOR
{
	union
    {
		UINT16	value;

	    struct
        {
	        UINT16	RPL		:  2;	// request privilege level (RPL)
	        UINT16	TI		:  1;	// Table indicator (0 = GDT, 1 = LDT)
	        UINT16	Index	: 13;	// indicate 8192 descriptors in GDT or LDT	        
        };
    };
} SEGMENT_SELECTOR, *PSEGMENT_SELECTOR;

//> segment descriptor
//> vol 3. 3.4.5
typedef struct SEGMENT_DESCRIPTOR
{
	union	
	{
		UINT64 value;

		struct 
		{
			UINT64	limit_0_15	: 16;	// 0-15,	(limit0)
			UINT64	base_0_15	: 16;	// 16-31	(base0)
			UINT64	base_16_23	: 8;	// 32-39,	(base1)

			// (attr0)
			UINT64	type		: 4;	// 40-43,	segment type
			UINT64	S			: 1;	// 44,		descriptor type. 0 = system, 1 = code or data
			UINT64	DPL			: 2;	// 45-46,	descriptor privilege level
			UINT64	P			: 1;	// 47,		segment present
			
			// (limit1_attr1)
			UINT64	limit_16_19	: 4;	// 48-51,				
			UINT64	AVL			: 1;	// 52,		available for use by system software
			UINT64	L			: 1;	// 53,		64bit code segment (IA-32e only)
			UINT64	DB			: 1;	// 54,		default operation size. (0 = 16 bit segment, 1 = 32bit segment)
			UINT64	G			: 1;	// 55,		granularity (0 = 1byte, 1 = 4Kb)
			
			// (base2)
			UINT64	base_24_31	: 8;	// 56-63,	
		} s1;

		struct 
		{
			UINT16	limit0;
			UINT16	base0;
			UINT8	base1;
			UINT8	attr0;
			UINT8	limit1_Attr1;
			UINT8	base2;
		} s2;
	};

} *PSEGMENT_DESCRIPTOR;

//> PSEGMENT_DESCRIPTOR 설정을 읽기 쉽게 하기 위해 정의한 상수 
//  e.g. 
//	SEGMENT_DESCRIPTOR::s2::attr0 & LA_STANDARD ( = 10000b ) : s1::S 비트를 체크, false = system , true = code or data
#define LA_ACCESSED		0x01
#define LA_READABLE		0x02    // for code segments
#define LA_WRITABLE		0x02    // for data segments
#define LA_CONFORMING	0x04    // for code segments
#define LA_EXPANDDOWN	0x04    // for data segments
#define LA_CODE			0x08
#define LA_STANDARD		0x10
#define LA_DPL_0		0x00
#define LA_DPL_1		0x20
#define LA_DPL_2		0x40
#define LA_DPL_3		0x60
#define LA_PRESENT		0x80

#define LA_LDT64		0x02
#define LA_ATSS64		0x09
#define LA_BTSS64		0x0b
#define LA_CALLGATE64	0x0c
#define LA_INTGATE64	0x0e
#define LA_TRAPGATE64	0x0f

#define HA_AVAILABLE	0x01
#define HA_LONG			0x02
#define HA_DB			0x04
#define HA_GRANULARITY	0x08


//> Debug Status Register - DR6
typedef struct _X86_DR6
{
	union
	{
		UINT32	value;
		struct 
		{
			UINT32 B0: 1;				// 0, 
			UINT32 B1: 1;				// 1, 
			UINT32 B2: 1;				// 2, 
			UINT32 B3: 1;				// 3, 
			UINT32 RESERVED_9_12: 9;	// 4, 0111 1111 1 (not used)	
			UINT32 BD: 1;				// 13,
			UINT32 BS: 1;				// 14,
			UINT32 BT: 1;				// 15,
			UINT32 RESERVED_16_31: 16;	// 16, 1111 1111 1111 1111 (not used)	
		};
	};
	
} X86_DR6, *PX86_DR6;

typedef struct _X64_DR6
{
	union
	{
		UINT64	value;
		struct 
		{
			UINT64 B0: 1;				// 0, 
			UINT64 B1: 1;				// 1, 
			UINT64 B2: 1;				// 2, 
			UINT64 B3: 1;				// 3, 
			UINT64 RESERVED_9_12: 9;	// 4, 0111 1111 1 (not used)	
			UINT64 BD: 1;				// 13,
			UINT64 BS: 1;				// 14,
			UINT64 BT: 1;				// 15,
			UINT64 RESERVED_16_31: 16;	// 16, 1111 1111 1111 1111 (not used)	
			UINT64 RESERVED_32_63: 32;	// 32, zero out
		};
	};
	
} X64_DR6, *PX64_DR6;

//> Debug Control Register - DR7
typedef struct _X86_DR7
{
	union
	{	
		UINT32	value;
		struct 
		{
			UINT32 L0: 1;				// 0,
			UINT32 G0: 1;				// 1, 
			UINT32 L1: 1;				// 2,
			UINT32 G1: 1;				// 3, 
			UINT32 L2: 1;				// 4, 
			UINT32 G2: 1;				// 5,
			UINT32 L3: 1;				// 6,
			UINT32 G3: 1;				// 7,
			UINT32 LE: 1;				// 8,
			UINT32 GE: 1;				// 9, 
			UINT32 RESERVED_10_12: 3;	// 10, 001 (not used)
			UINT32 GD: 1;				// 13,
			UINT32 RESERVED_14_15: 2;	// 14, 00 (not used)
			UINT32 RW0 : 2;				// 16,
			UINT32 LEN0: 2;				// 18,
			UINT32 RW1 : 2;				// 20,
			UINT32 LEN1: 2;				// 22,
			UINT32 RW2 : 2;				// 24, 
			UINT32 LEN2: 2;				// 26, 
			UINT32 RW3 : 2;				// 29, 
			UINT32 LEN3: 2;				// 30
		};	
	};
	
} X86_DR7, *PX86_DR7;

typedef struct _X64_DR7
{
	union
	{	
		UINT64	value;
		struct 
		{
			UINT64 L0: 1;				// 0,
			UINT64 G0: 1;				// 1, 
			UINT64 L1: 1;				// 2,
			UINT64 G1: 1;				// 3, 
			UINT64 L2: 1;				// 4, 
			UINT64 G2: 1;				// 5,
			UINT64 L3: 1;				// 6,
			UINT64 G3: 1;				// 7,
			UINT64 LE: 1;				// 8,
			UINT64 GE: 1;				// 9, 
			UINT64 RESERVED_10_12: 3;	// 10, 001 (not used)
			UINT64 GD: 1;				// 13,
			UINT64 RESERVED_14_15: 2;	// 14, 00 (not used)
			UINT64 RW0 : 2;				// 16,
			UINT64 LEN0: 2;				// 18,
			UINT64 RW1 : 2;				// 20,
			UINT64 LEN1: 2;				// 22,
			UINT64 RW2 : 2;				// 24, 
			UINT64 LEN2: 2;				// 26, 
			UINT64 RW3 : 2;				// 29, 
			UINT64 LEN3: 2;				// 30,
			UINT64 RESERVED_32_63: 32;	// 32, zero out
		};	
	};
	
} X64_DR7, *PX64_DR7;


#ifdef _M_AMD64
	typedef X64_DR6	DR6;
	typedef X64_DR7	DR7;
#elif defined(_M_IX86)
	typedef X86_DR6	DR6;
	typedef X86_DR7	DR7;
#else
	#error !need to write code for this architecture
#endif

//> x86 page directory entry 
typedef struct _X86_PDE
{
	union
	{
		struct 
		{
			UINT32 value;
		};

		struct 
		{
			UINT32 p		: 1;	// [0], present (1 : present)
			UINT32 rw		: 1;	// [1], read / write (0: write not allowd)
			UINT32 us		: 1;	// [2], user / supervisor (0: user mode access not allowd)
			UINT32 pwt		: 1;	// [3], page-level write through
			UINT32 pcd		: 1;	// [4], page-level cache disabled
			UINT32 a		: 1;	// [5], accessed
			UINT32 d_ignored: 1;	// [6], dirty (ignored)
			UINT32 ps		: 1;	// [7], page size (0 : 4kb page) ( CR4.pse == 1 이면 반드시 1 -> 4mb page)
			UINT32 g		: 1;	// [8], global page
			UINT32 avail	: 3;	// [9 -11], available to programmer
			UINT32 pfn		: 20;	// [12-31], page frame number
		};
	};
} X86_PDE, *PX86_PDE;

//> x86 page table entry
typedef struct _X86_PTE
{
	union
	{
		struct 
		{
			UINT32 value;
		};

		struct 
		{
			UINT32 p		: 1;	// [0], present (1 : present)
			UINT32 rw		: 1;	// [1], read / write (0: write not allowd)
			UINT32 us		: 1;	// [2], user / supervisor (0: user mode access not allowd)
			UINT32 pwt		: 1;	// [3], page-level write through
			UINT32 pcd		: 1;	// [4], page-level cache disabled
			UINT32 a		: 1;	// [5], accessed
			UINT32 d		: 1;	// [6], dirty
			UINT32 unused	: 1;	// [7], 0
			UINT32 g		: 1;	// [8], global page
			UINT32 avail	: 3;	// [9 -11], available to programmer
			UINT32 pfn		: 20;	// [12-31], page frame number

		};
	};
} X86_PTE, *PX86_PTE;

















//> cpuid instruction
//	input: eax=1, ecx value
 
typedef struct CPUID_1_ECX
{
	unsigned SSE3		:1;		// SSE3 Extensions
	unsigned RES1		:2;
	unsigned MONITOR	:1;		// MONITOR/WAIT
	unsigned DS_CPL		:1;		// CPL qualified Debug Store
	unsigned VMX		:1;		// Virtual Machine Technology
	unsigned RES2		:1;
	unsigned EST		:1;		// Enhanced Intel?Speedstep Technology
	unsigned TM2		:1;		// Thermal monitor 2
	unsigned SSSE3		:1;		// SSSE3 extensions
	unsigned CID		:1;		// L1 context ID
	unsigned RES3		:2;
	unsigned CX16		:1;		// CMPXCHG16B
	unsigned xTPR		:1;		// Update control
	unsigned PDCM		:1;		// Performance/Debug capability MSR
	unsigned RES4		:2;
	unsigned DCA		:1;
	unsigned RES5		:13;

} CPUID_1_ECX;




//> MSR index
//  Intel architecture manaual 3B appendix B Model-Specific Registers(MSRs)
//  rdmsr.[ecx=index]

#define MSR_IA32_FEATURE_CONTROL		0x03A
#define MSR_IA32_SYSENTER_CS            0x174
#define MSR_IA32_SYSENTER_ESP           0x175
#define MSR_IA32_SYSENTER_EIP           0x176
#define IA32_DEBUGCTL_MSR				0x1D9

#define	MSR_IA32_FS_BASE    			0xc0000100
#define	MSR_IA32_GS_BASE	            0xc0000101





//> msr structure (eax:edx)
typedef struct MSR
{
	UINT32 low;
	UINT32 high;
} *PMSR;

//> rdmsr.[ecx=MSR_IA32_FEATURE_CONTROL]
typedef struct IA32_FEATURE_CONTROL 
{
	unsigned Lock			:1;		// Bit 0 is the lock bit - cannot be modified once lock is set
	unsigned Reserved1		:1;		// Undefined
	unsigned EnableVmxon	:1;		// Bit 2. If this bit is clear, VMXON causes a general protection exception
	unsigned Reserved2		:29;	// Undefined
	unsigned Reserved3		:32;	// Undefined

} *PIA32_FEATURE_CONTROL;

//> volume 3B 21.4.1 Guest Register State
//	attributes for segment selector. 
//	this is copy of bit 40:47, 52:55 of the segment descriptor (->SEGMENT_DESCRIPTOR)
typedef struct _GUEST_SEGMENT_ATTRIBUTE
{
	union
	{
		UINT16	value;
		
		struct 
		{
			UINT16	type		: 4;	// 0, 40-43,	segment type
			UINT16	S			: 1;	// 4, 44,		descriptor type. 0 = system, 1 = code or data
			UINT16	DPL			: 2;	// 5, 45-46,	descriptor privilege level
			UINT16	P			: 1;	// 7, 47,		segment present
			//>
			UINT16	AVL			: 1;	// 8, 52,		available for use by system software
			UINT16	L			: 1;	// 9, 53,		64bit code segment (IA-32e only)
			UINT16	DB			: 1;	// 10, 54,		default operation size. (0 = 16 bit segment, 1 = 32bit segment)
			UINT16	G			: 1;	// 11, 55,		granularity (0 = 1byte, 1 = 4Kb)
			UINT16  Gap			: 4;
		};
	};
} GUEST_SEGMENT_ATTRIBUTE, *PGUEST_SEGMENT_ATTRIBUTE;

typedef struct GUEST_SEGMENT_SELECTOR
{
	SEGMENT_SELECTOR			selector;		// 16 bit 
	GUEST_SEGMENT_ATTRIBUTE		attribute;		// 16 bit
	UINT32						limit;
	UINT64						base;
	
} *PGUEST_SEGMENT_SELECTOR;


//> VMX Capability Reporting Facility By MSR index
//	Intel architecture manaual 3B appendix G 
#define MSR_IA32_VMX_BASIC					0x480
#define MSR_IA32_VMX_PINBASED_CTLS			0x481
#define MSR_IA32_VMX_PROCBASED_CTLS			0x482
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS		0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS	0x48E

#define MSR_IA32_VMX_EXIT_CTLS				0x483
#define MSR_IA32_VMX_TRUE_EXIT_CTLS			0x48F

#define MSR_IA32_VMX_ENTRY_CTLS				0x484
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS		0x490

#define MSR_IA32_VMX_MISC					0x485
#define MSR_IA32_VMX_CR0_FIXED0				0x486
#define MSR_IA32_VMX_CR0_FIXED1				0x487
#define MSR_IA32_VMX_CR4_FIXED0				0x488
#define MSR_IA32_VMX_CR4_FIXED1				0x489
#define MSR_IA32_VMX_VMCS_ENUM				0x48A
#define MSR_IA32_VMX_EPT_VPID_CAP			0x48C

//> rdmsr.[ecx=MSR_IA32_VMX_BASIC]
typedef struct IA32_VMX_BASIC
{
	unsigned RevId			:32;	// 0..31,	contain the VMCS revision identifier
	unsigned szVmxOnRegion  :12;	// 32..43,	report # of bytes for VMXON region ( maxmum=4096 )
	unsigned RegionClear	:1;		// 44,		set only if bits 32-43 are clear
	unsigned Reserved1		:3;		// 45..47,	Undefined
	unsigned PhyAddrWidth	:1;		// 48,		Physical address width for referencing VMXON, VMCS, etc.
	unsigned DualMon		:1;		// 49,		Reports whether the processor supports dual-monitor
									//			treatment of SMI and SMM
	unsigned MemType		:4;		// 50..53,	Memory type that the processor uses to access the VMCS
	unsigned VmExitReport	:1;		// 54,		Reports weather the procesor reports info in the VM-exit
									//			instruction information field on VM exits due to execution
									//			of the INS and OUTS instructions
	unsigned Reserved2		:9;		// 55..63,	Undefined

} *PIA32_VMX_BASIC;

//> Exit reasons
#define EXIT_REASON_EXCEPTION_NMI        0
#define EXIT_REASON_EXTERNAL_INTERRUPT   1
#define EXIT_REASON_TRIPLE_FAULT         2
#define EXIT_REASON_INIT                 3
#define EXIT_REASON_SIPI                 4
#define EXIT_REASON_IO_SMI               5
#define EXIT_REASON_OTHER_SMI            6
#define EXIT_REASON_PENDING_INTERRUPT    7
#define EXIT_REASON_TASK_SWITCH          9
#define EXIT_REASON_CPUID               10
#define EXIT_REASON_HLT                 12
#define EXIT_REASON_INVD                13
#define EXIT_REASON_INVLPG              14
#define EXIT_REASON_RDPMC               15
#define EXIT_REASON_RDTSC               16
#define EXIT_REASON_RSM                 17
#define EXIT_REASON_VMCALL              18
#define EXIT_REASON_VMCLEAR             19
#define EXIT_REASON_VMLAUNCH            20
#define EXIT_REASON_VMPTRLD             21
#define EXIT_REASON_VMPTRST             22
#define EXIT_REASON_VMREAD              23
#define EXIT_REASON_VMRESUME            24
#define EXIT_REASON_VMWRITE             25
#define EXIT_REASON_VMXOFF              26
#define EXIT_REASON_VMXON               27
#define EXIT_REASON_CR_ACCESS           28
#define EXIT_REASON_DR_ACCESS           29
#define EXIT_REASON_IO_INSTRUCTION      30
#define EXIT_REASON_MSR_READ            31
#define EXIT_REASON_MSR_WRITE           32
#define EXIT_REASON_INVALID_GUEST_STATE 33
#define EXIT_REASON_MSR_LOADING         34
#define EXIT_REASON_MWAIT_INSTRUCTION   36
#define EXIT_REASON_MONITOR_INSTRUCTION 39
#define EXIT_REASON_PAUSE_INSTRUCTION   40
#define EXIT_REASON_MACHINE_CHECK       41
#define EXIT_REASON_TPR_BELOW_THRESHOLD 43

//> VM-execution control bits
#define PROC_BASED_HLT_EXITING			7
#define PROC_BASED_USE_IOBITMAP			25

//> VM-exit control bits
#define VM_EXIT_ACK_INTERRUPT_ON_EXIT   15

//> Exception/NMI-related information
#define INTR_INFO_VECTOR_MASK           0xff            /* bits 0:7 */
#define INTR_INFO_INTR_TYPE_MASK        0x700           /* bits 8:10 */
#define INTR_INFO_DELIVER_CODE_MASK     0x800           /* bit 11 must be set to push error code on guest stack*/
#define INTR_INFO_VALID_MASK            0x80000000      /* bit 31 must be set to identify valid events */
#define INTR_TYPE_EXT_INTR              (0 << 8)        /* external interrupt */
#define INTR_TYPE_NMI                   (2 << 8)        /* NMI */
#define INTR_TYPE_HW_EXCEPTION          (3 << 8)        /* hardware exception */
#define INTR_TYPE_SW_EXCEPTION          (6 << 8)        /* software exception */


//> VMCS encoding
enum
{
  GUEST_ES_SELECTOR = 0x00000800,
  GUEST_CS_SELECTOR = 0x00000802,
  GUEST_SS_SELECTOR = 0x00000804,
  GUEST_DS_SELECTOR = 0x00000806,
  GUEST_FS_SELECTOR = 0x00000808,
  GUEST_GS_SELECTOR = 0x0000080a,
  GUEST_LDTR_SELECTOR = 0x0000080c,
  GUEST_TR_SELECTOR = 0x0000080e,
  HOST_ES_SELECTOR = 0x00000c00,
  HOST_CS_SELECTOR = 0x00000c02,
  HOST_SS_SELECTOR = 0x00000c04,
  HOST_DS_SELECTOR = 0x00000c06,
  HOST_FS_SELECTOR = 0x00000c08,
  HOST_GS_SELECTOR = 0x00000c0a,
  HOST_TR_SELECTOR = 0x00000c0c,
  IO_BITMAP_A = 0x00002000,
  IO_BITMAP_A_HIGH = 0x00002001,
  IO_BITMAP_B = 0x00002002,
  IO_BITMAP_B_HIGH = 0x00002003,
  MSR_BITMAP = 0x00002004,
  MSR_BITMAP_HIGH = 0x00002005,
  VM_EXIT_MSR_STORE_ADDR = 0x00002006,
  VM_EXIT_MSR_STORE_ADDR_HIGH = 0x00002007,
  VM_EXIT_MSR_LOAD_ADDR = 0x00002008,
  VM_EXIT_MSR_LOAD_ADDR_HIGH = 0x00002009,
  VM_ENTRY_MSR_LOAD_ADDR = 0x0000200a,
  VM_ENTRY_MSR_LOAD_ADDR_HIGH = 0x0000200b,
  TSC_OFFSET = 0x00002010,
  TSC_OFFSET_HIGH = 0x00002011,
  VIRTUAL_APIC_PAGE_ADDR = 0x00002012,
  VIRTUAL_APIC_PAGE_ADDR_HIGH = 0x00002013,
  VMCS_LINK_POINTER = 0x00002800,
  VMCS_LINK_POINTER_HIGH = 0x00002801,
  GUEST_IA32_DEBUGCTL = 0x00002802,
  GUEST_IA32_DEBUGCTL_HIGH = 0x00002803,
  PIN_BASED_VM_EXEC_CONTROL = 0x00004000,
  CPU_BASED_VM_EXEC_CONTROL = 0x00004002,
  EXCEPTION_BITMAP = 0x00004004,
  PAGE_FAULT_ERROR_CODE_MASK = 0x00004006,
  PAGE_FAULT_ERROR_CODE_MATCH = 0x00004008,
  CR3_TARGET_COUNT = 0x0000400a,
  VM_EXIT_CONTROLS = 0x0000400c,
  VM_EXIT_MSR_STORE_COUNT = 0x0000400e,
  VM_EXIT_MSR_LOAD_COUNT = 0x00004010,
  VM_ENTRY_CONTROLS = 0x00004012,
  VM_ENTRY_MSR_LOAD_COUNT = 0x00004014,
  VM_ENTRY_INTR_INFO_FIELD = 0x00004016,
  VM_ENTRY_EXCEPTION_ERROR_CODE = 0x00004018,
  VM_ENTRY_INSTRUCTION_LEN = 0x0000401a,
  TPR_THRESHOLD = 0x0000401c,
  SECONDARY_VM_EXEC_CONTROL = 0x0000401e,
  VM_INSTRUCTION_ERROR = 0x00004400,
  VM_EXIT_REASON = 0x00004402,
  VM_EXIT_INTR_INFO = 0x00004404,
  VM_EXIT_INTR_ERROR_CODE = 0x00004406,
  IDT_VECTORING_INFO_FIELD = 0x00004408,
  IDT_VECTORING_ERROR_CODE = 0x0000440a,
  VM_EXIT_INSTRUCTION_LEN = 0x0000440c,
  VMX_INSTRUCTION_INFO = 0x0000440e,
  GUEST_ES_LIMIT = 0x00004800,
  GUEST_CS_LIMIT = 0x00004802,
  GUEST_SS_LIMIT = 0x00004804,
  GUEST_DS_LIMIT = 0x00004806,
  GUEST_FS_LIMIT = 0x00004808,
  GUEST_GS_LIMIT = 0x0000480a,
  GUEST_LDTR_LIMIT = 0x0000480c,
  GUEST_TR_LIMIT = 0x0000480e,
  GUEST_GDTR_LIMIT = 0x00004810,
  GUEST_IDTR_LIMIT = 0x00004812,
  GUEST_ES_AR_BYTES = 0x00004814,
  GUEST_CS_AR_BYTES = 0x00004816,
  GUEST_SS_AR_BYTES = 0x00004818,
  GUEST_DS_AR_BYTES = 0x0000481a,
  GUEST_FS_AR_BYTES = 0x0000481c,
  GUEST_GS_AR_BYTES = 0x0000481e,
  GUEST_LDTR_AR_BYTES = 0x00004820,
  GUEST_TR_AR_BYTES = 0x00004822,
  GUEST_INTERRUPTIBILITY_INFO = 0x00004824,
  GUEST_ACTIVITY_STATE = 0x00004826,
  GUEST_SM_BASE = 0x00004828,
  GUEST_SYSENTER_CS = 0x0000482A,
  HOST_IA32_SYSENTER_CS = 0x00004c00,
  CR0_GUEST_HOST_MASK = 0x00006000,
  CR4_GUEST_HOST_MASK = 0x00006002,
  CR0_READ_SHADOW = 0x00006004,
  CR4_READ_SHADOW = 0x00006006,
  CR3_TARGET_VALUE0 = 0x00006008,
  CR3_TARGET_VALUE1 = 0x0000600a,
  CR3_TARGET_VALUE2 = 0x0000600c,
  CR3_TARGET_VALUE3 = 0x0000600e,
  EXIT_QUALIFICATION = 0x00006400,
  GUEST_LINEAR_ADDRESS = 0x0000640a,
  GUEST_CR0 = 0x00006800,
  GUEST_CR3 = 0x00006802,
  GUEST_CR4 = 0x00006804,
  GUEST_ES_BASE = 0x00006806,
  GUEST_CS_BASE = 0x00006808,
  GUEST_SS_BASE = 0x0000680a,
  GUEST_DS_BASE = 0x0000680c,
  GUEST_FS_BASE = 0x0000680e,
  GUEST_GS_BASE = 0x00006810,
  GUEST_LDTR_BASE = 0x00006812,
  GUEST_TR_BASE = 0x00006814,
  GUEST_GDTR_BASE = 0x00006816,
  GUEST_IDTR_BASE = 0x00006818,
  GUEST_DR7 = 0x0000681a,
  GUEST_RSP = 0x0000681c,
  GUEST_RIP = 0x0000681e,
  GUEST_RFLAGS = 0x00006820,
  GUEST_PENDING_DBG_EXCEPTIONS = 0x00006822,
  GUEST_SYSENTER_ESP = 0x00006824,
  GUEST_SYSENTER_EIP = 0x00006826,
  HOST_CR0 = 0x00006c00,
  HOST_CR3 = 0x00006c02,
  HOST_CR4 = 0x00006c04,
  HOST_FS_BASE = 0x00006c06,
  HOST_GS_BASE = 0x00006c08,
  HOST_TR_BASE = 0x00006c0a,
  HOST_GDTR_BASE = 0x00006c0c,
  HOST_IDTR_BASE = 0x00006c0e,
  HOST_IA32_SYSENTER_ESP = 0x00006c10,
  HOST_IA32_SYSENTER_EIP = 0x00006c12,
  HOST_RSP = 0x00006c14,
  HOST_RIP = 0x00006c16,
};

typedef struct _VMCS
{
  ULONG64 GUEST_ES_SELECTOR;
  ULONG64 GUEST_CS_SELECTOR;
  ULONG64 GUEST_SS_SELECTOR;
  ULONG64 GUEST_DS_SELECTOR;
  ULONG64 GUEST_FS_SELECTOR;
  ULONG64 GUEST_GS_SELECTOR;
  ULONG64 GUEST_LDTR_SELECTOR;
  ULONG64 GUEST_TR_SELECTOR;
  ULONG64 HOST_ES_SELECTOR;
  ULONG64 HOST_CS_SELECTOR;
  ULONG64 HOST_SS_SELECTOR;
  ULONG64 HOST_DS_SELECTOR;
  ULONG64 HOST_FS_SELECTOR;
  ULONG64 HOST_GS_SELECTOR;
  ULONG64 HOST_TR_SELECTOR;
  ULONG64 IO_BITMAP_A;
  ULONG64 IO_BITMAP_A_HIGH;
  ULONG64 IO_BITMAP_B;
  ULONG64 IO_BITMAP_B_HIGH;
  ULONG64 MSR_BITMAP;
  ULONG64 MSR_BITMAP_HIGH;
  ULONG64 VM_EXIT_MSR_STORE_ADDR;
  ULONG64 VM_EXIT_MSR_STORE_ADDR_HIGH;
  ULONG64 VM_EXIT_MSR_LOAD_ADDR;
  ULONG64 VM_EXIT_MSR_LOAD_ADDR_HIGH;
  ULONG64 VM_ENTRY_MSR_LOAD_ADDR;
  ULONG64 VM_ENTRY_MSR_LOAD_ADDR_HIGH;
  ULONG64 TSC_OFFSET;
  ULONG64 TSC_OFFSET_HIGH;
  ULONG64 VIRTUAL_APIC_PAGE_ADDR;
  ULONG64 VIRTUAL_APIC_PAGE_ADDR_HIGH;
  ULONG64 VMCS_LINK_POINTER;
  ULONG64 VMCS_LINK_POINTER_HIGH;
  ULONG64 GUEST_IA32_DEBUGCTL;
  ULONG64 GUEST_IA32_DEBUGCTL_HIGH;
  ULONG64 PIN_BASED_VM_EXEC_CONTROL;
  ULONG64 CPU_BASED_VM_EXEC_CONTROL;
  ULONG64 EXCEPTION_BITMAP;
  ULONG64 PAGE_FAULT_ERROR_CODE_MASK;
  ULONG64 PAGE_FAULT_ERROR_CODE_MATCH;
  ULONG64 CR3_TARGET_COUNT;
  ULONG64 VM_EXIT_CONTROLS;
  ULONG64 VM_EXIT_MSR_STORE_COUNT;
  ULONG64 VM_EXIT_MSR_LOAD_COUNT;
  ULONG64 VM_ENTRY_CONTROLS;
  ULONG64 VM_ENTRY_MSR_LOAD_COUNT;
  ULONG64 VM_ENTRY_INTR_INFO_FIELD;
  ULONG64 VM_ENTRY_EXCEPTION_ERROR_CODE;
  ULONG64 VM_ENTRY_INSTRUCTION_LEN;
  ULONG64 TPR_THRESHOLD;
  ULONG64 SECONDARY_VM_EXEC_CONTROL;
  ULONG64 VM_INSTRUCTION_ERROR;
  ULONG64 VM_EXIT_REASON;
  ULONG64 VM_EXIT_INTR_INFO;
  ULONG64 VM_EXIT_INTR_ERROR_CODE;
  ULONG64 IDT_VECTORING_INFO_FIELD;
  ULONG64 IDT_VECTORING_ERROR_CODE;
  ULONG64 VM_EXIT_INSTRUCTION_LEN;
  ULONG64 VMX_INSTRUCTION_INFO;
  ULONG64 GUEST_ES_LIMIT;
  ULONG64 GUEST_CS_LIMIT;
  ULONG64 GUEST_SS_LIMIT;
  ULONG64 GUEST_DS_LIMIT;
  ULONG64 GUEST_FS_LIMIT;
  ULONG64 GUEST_GS_LIMIT;
  ULONG64 GUEST_LDTR_LIMIT;
  ULONG64 GUEST_TR_LIMIT;
  ULONG64 GUEST_GDTR_LIMIT;
  ULONG64 GUEST_IDTR_LIMIT;
  ULONG64 GUEST_ES_AR_BYTES;
  ULONG64 GUEST_CS_AR_BYTES;
  ULONG64 GUEST_SS_AR_BYTES;
  ULONG64 GUEST_DS_AR_BYTES;
  ULONG64 GUEST_FS_AR_BYTES;
  ULONG64 GUEST_GS_AR_BYTES;
  ULONG64 GUEST_LDTR_AR_BYTES;
  ULONG64 GUEST_TR_AR_BYTES;
  ULONG64 GUEST_INTERRUPTIBILITY_INFO;
  ULONG64 GUEST_ACTIVITY_STATE;
  ULONG64 GUEST_SM_BASE;
  ULONG64 GUEST_SYSENTER_CS;
  ULONG64 HOST_IA32_SYSENTER_CS;
  ULONG64 CR0_GUEST_HOST_MASK;
  ULONG64 CR4_GUEST_HOST_MASK;
  ULONG64 CR0_READ_SHADOW;
  ULONG64 CR4_READ_SHADOW;
  ULONG64 CR3_TARGET_VALUE0;
  ULONG64 CR3_TARGET_VALUE1;
  ULONG64 CR3_TARGET_VALUE2;
  ULONG64 CR3_TARGET_VALUE3;
  ULONG64 EXIT_QUALIFICATION;
  ULONG64 GUEST_LINEAR_ADDRESS;
  ULONG64 GUEST_CR0;
  ULONG64 GUEST_CR3;
  ULONG64 GUEST_CR4;
  ULONG64 GUEST_ES_BASE;
  ULONG64 GUEST_CS_BASE;
  ULONG64 GUEST_SS_BASE;
  ULONG64 GUEST_DS_BASE;
  ULONG64 GUEST_FS_BASE;
  ULONG64 GUEST_GS_BASE;
  ULONG64 GUEST_LDTR_BASE;
  ULONG64 GUEST_TR_BASE;
  ULONG64 GUEST_GDTR_BASE;
  ULONG64 GUEST_IDTR_BASE;
  ULONG64 GUEST_DR7;
  ULONG64 GUEST_RSP;
  ULONG64 GUEST_RIP;
  ULONG64 GUEST_RFLAGS;
  ULONG64 GUEST_PENDING_DBG_EXCEPTIONS;
  ULONG64 GUEST_SYSENTER_ESP;
  ULONG64 GUEST_SYSENTER_EIP;
  ULONG64 HOST_CR0;
  ULONG64 HOST_CR3;
  ULONG64 HOST_CR4;
  ULONG64 HOST_FS_BASE;
  ULONG64 HOST_GS_BASE;
  ULONG64 HOST_TR_BASE;
  ULONG64 HOST_GDTR_BASE;
  ULONG64 HOST_IDTR_BASE;
  ULONG64 HOST_IA32_SYSENTER_ESP;
  ULONG64 HOST_IA32_SYSENTER_EIP;
  ULONG64 HOST_RSP;
  ULONG64 HOST_RIP;
} VMCS, *PVMCS;

#pragma pack()


#pragma warning(default: 4201)
#pragma warning(default: 4214)


#endif//_arch_header_