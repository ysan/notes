#ifndef _DEFINES_H_
#define _DEFINES_H_

/*
 * 定数定義
 */

#ifndef NULL
#define NULL					((void*)0)
#endif

typedef unsigned char			BOOL;
#define FALSE					((BOOL)0)
#define TRUE					((BOOL)1)


#define INTR_VECT_NUM_MAX		(64)

#define RAM_BUFF_SECTION_SIZE	(0x1000)	/* 4KB */
#define DRAM_SECTION_SIZE		(0x200000)	/* 2MB */

#define BOOTMSG					"H83069 bootloader start\n"


/*
 * 型定義
 */
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

typedef void (*VOID_FUNC)( void );


/*
 * 外部宣言
 */
/* リンカスクリプトで定義しているシンボル */
extern BYTE rodata_start;
extern BYTE rodata_end;
extern BYTE data_start;
extern BYTE data_end;
extern BYTE bss_start;
extern BYTE bss_end;
extern BYTE softvec;
extern BYTE ram_buff;
extern BYTE dram_start;

/* アセンブラ関数 */
extern void start( void );
extern void intr_softerr( void );
extern void intr_syscall( void );
extern void intr_sci1recv( void );
extern void intr_timer0( void );
extern void intr_timer2( void );

#endif
