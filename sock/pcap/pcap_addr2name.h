#ifndef __PCAP_ADDR2NAME_H
#define __PCAP_ADDR2NAME_H


#define NAMELIST_HASHSIZE		(8) /* 2のべき乗を指定 */
#define NAMELIST_MALLOC_SIZE	(3)
#define NAME_SIZE				(128)

#define CALC_HASH( x ) ( x & (NAMELIST_HASHSIZE-1) )


typedef struct namelist {
	uint32_t nAddr;
	char szName[NAME_SIZE];
	struct namelist *pNext;
	struct namelist *pNextMalloc;
} ST_NAMELIST;



extern void InitNameList( void );
extern int GetName( uint32_t, char*, size_t);
extern void RefNameList( void );
extern void FreeAllMallocList( void );

#endif
