#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_


/*
 * 定数定義
 */
#define ELF_HEADER_MAGIC_SIZE		(4)
#define ELF_HEADER_RESERVE_SIZE		(7)

/*
 * 型定義
 */
typedef struct elf_header {

	struct {
		BYTE bMagic[ ELF_HEADER_MAGIC_SIZE ];
		BYTE bClass;
		BYTE bFormat;
		BYTE bVersion;
		BYTE bAbi;
		BYTE bAbiVersion;
		BYTE bReserve[ ELF_HEADER_RESERVE_SIZE ];
	} id;

	WORD wType;
	WORD wArch;
	DWORD dwVersion;
	DWORD dwEntryPoint;
	DWORD dwProgramHeaderOffset;
	DWORD dwSectionHeaderOffset;
	DWORD dwFlags;
	WORD wHeaderSize;
	WORD wProgramHeaderSize;
	WORD wProgramHeaderNum;
	WORD wSectionHeaderSize;
	WORD wSectionHeaderNum;
	WORD wSectionNameIndex;

} ST_ELF_HEADER;

typedef struct elf_program_header {
	DWORD dwType;
	DWORD dwOffset;
	DWORD dwVirtualAddr;
	DWORD dwPhysicalAddr;
	DWORD dwFileSize;
	DWORD dwMemorySize;
	DWORD dwFlags;
	DWORD dwAlign;
} ST_ELF_PROGRAM_HEADER;


/*
 * 外部宣言
 */
extern BOOL Boot( const void *pBuff );


#endif
