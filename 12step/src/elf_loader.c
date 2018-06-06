#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defines.h"
#include "elf_loader.h"


/*
 * プロトタイプ宣言
 */
static BYTE CheckElfHeader( const ST_ELF_HEADER *pstElfHeader );
static BYTE LoadProgramHeaderTable( const ST_ELF_HEADER *pstElfHeader );
static BYTE GetEntryPoint( const void *pArgBuff, DWORD *pdwEntryPoint );

/*
 * ELFヘッダのチェック
 */
static BOOL CheckElfHeader( const ST_ELF_HEADER *pstElfHeader )
{
	if ( !pstElfHeader ) {
		return FALSE;
	}

	if ( memcmp( pstElfHeader->id.bMagic, "\x7f" "ELF", 4 ) ) {
		return FALSE;
	}

	/* ELF32 */
	if ( pstElfHeader->id.bClass != 1 ) {
		return FALSE;
	}

	/* Big endian */
	if ( pstElfHeader->id.bFormat != 2 ) {
		return FALSE;
	}

	/* version 1 */
	if ( pstElfHeader->id.bVersion != 1 ) {
		return FALSE;
	}

	/* Executable file */
	if ( pstElfHeader->wType != 2 ) {
		return FALSE;
	}

	/* version 1 */
	if ( pstElfHeader->dwVersion != 1 ) {
		return FALSE;
	}

	/* H8/300 or H8/300H */
	if ( ( pstElfHeader->wArch != 46 ) && ( pstElfHeader->wArch != 47 ) ) {
		return FALSE;
	}

	return TRUE;
}

/*
 * セグメント単位でのロード
 */
static BOOL LoadProgramHeaderTable( const ST_ELF_HEADER *pstElfHeader )
{
	if ( !pstElfHeader ) {
		return FALSE;
	}

	BYTE i;
	ST_ELF_PROGRAM_HEADER *pstElfProgramHeader;

	for ( i = 0; i < pstElfHeader->wProgramHeaderNum; i ++ ) {

		/* プログラムヘッダを取得 */
		pstElfProgramHeader = (ST_ELF_PROGRAM_HEADER*) ( (BYTE*)pstElfHeader +
												pstElfHeader->dwProgramHeaderOffset +
												pstElfHeader->wProgramHeaderSize * i );

		/* ロード可能なセグメントでなければ次へ */
		if ( pstElfProgramHeader->dwType != 1 ) {
			continue;
		}

		/* ロード実行 */
		memcpy (
			(void*)( pstElfProgramHeader->dwPhysicalAddr ),
			pstElfHeader + pstElfProgramHeader->dwOffset,
			pstElfProgramHeader->dwFileSize
		);

		memset (
			(void*)( pstElfProgramHeader->dwPhysicalAddr + pstElfProgramHeader->dwFileSize ),
			0x00,
			pstElfProgramHeader->dwMemorySize - pstElfProgramHeader->dwFileSize
		);
	}

	return TRUE;
}

/*
 * ELF形式データのロード
 * エントリポイントの取得
 */
static BOOL GetEntryPoint( const void *pBuff, DWORD *pdwEntryPoint )
{
	if ( ( !pBuff ) || ( !pdwEntryPoint ) ) {
		return FALSE;
	}

	ST_ELF_HEADER *pstElfHeader = (ST_ELF_HEADER*)pBuff;

	/* ELFヘッダのチェック */
	if ( !CheckElfHeader( pstElfHeader ) ) {
		return FALSE;
	}

	/* セグメント単位でのロード */
	if ( !LoadProgramHeaderTable( pstElfHeader ) ) {
		return FALSE;
	}

	*pdwEntryPoint = pstElfHeader->dwEntryPoint;

	return TRUE;
}

/*
 * エントリポイントからの起動
 */
BOOL Boot( const void *pBuff )
{
	DWORD dwEntryPoint;
	VOID_FUNC pFunc = NULL;

	if ( !GetEntryPoint( pBuff, &dwEntryPoint ) ) {
		return FALSE;
	}

	pFunc = (VOID_FUNC)dwEntryPoint;

	if ( pFunc ) {
		pFunc();
	}


	return TRUE;
}
