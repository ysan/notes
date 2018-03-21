#ifndef _UTIL_H_
#define _UTIL_H_


/*
 * Type define
 */
typedef union {
	unsigned int nVal;
	struct {	/* little endian */
		unsigned int fraction : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} ST_FLOAT;
} UN_FLOAT;

typedef union {
	unsigned long long nVal;
	struct {	/* little endian */
		unsigned long long fraction : 52;
		unsigned long long exponent : 11;
		unsigned long long sign : 1;
	} ST_DOUBLE;
} UN_DOUBLE;


/*
 * External
 */
extern uint32_t Double2FixedOnlyFraction( double arg );
extern double Fixed2DoubleOnlyFraction( uint32_t arg );
extern struct serv_info *GetServerList( void );
extern void DestroyServerList( struct serv_info *p );

#endif
