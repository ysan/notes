#ifndef _TS_COMMON_DEF_H_
#define _TS_COMMON_DEF_H_


#define TS_PACKET_LEN					(188)
#define SYNC_BYTE						(0x47)
#define TS_HEADER_LEN					(4)
#define SECTION_HEADER_LEN				(8)
#define SECTION_HEADER_FIX_LEN			(5) // after section_length
#define SECTION_SHORT_HEADER_LEN		(3) // section_length include, before
#define SECTION_CRC32_LEN				(4)

#define PMT_FIX_LEN						(4) // reserved             3  bslbf
											// PCR_PID             13  uimsbf
											// reserved             4  bslbf
											// program_info_length 12 uimsbf

typedef enum {
	EN_DESCRIPTOR_PARSE_SATGE__TAG,
	EN_DESCRIPTOR_PARSE_SATGE__LEN,
	EN_DESCRIPTOR_PARSE_SATGE__DATA,
} EN_DESCRIPTOR_PARSE_SATGE;

typedef enum {
	EN_PSI_SI_TYPE__PAT = 0,
	EN_PSI_SI_TYPE__PMT,
	EN_PSI_SI_TYPE__TOT,

	EN_PSI_SI_TYPE__NUM,
	EN_PSI_SI_TYPE__BLANK,
} EN_PSI_SI_TYPE;


typedef struct {
	uint8_t sync;							/*  0- 7 :  8 bits */
	uint8_t transport_error_indicator;		/*  8- 8 :  1 bit  */
	uint8_t payload_unit_start_indicator;	/*  9- 9 :  1 bit  */
	uint8_t transport_priority;				/* 10-10 :  1 bits */
	uint16_t pid;							/* 11-23 : 13 bits */
	uint8_t transport_scrambling_control;	/* 24-25 :  2 bits */
	uint8_t adaptation_field_control;		/* 26-27 :  2 bits */
	uint8_t continuity_counter;				/* 28-31 :  4 bits */
} ST_TS_HEADER;


typedef struct {
	uint8_t table_id;						/*  0- 7 :  8 bits */
	uint8_t section_syntax_indicator;		/*  8- 8 :  1 bit  */
	uint8_t private_indicator;				/*  9- 9 :  1 bit  */
	// reserved								/* 10-11 :  2 bits */
	uint16_t section_length;				/* 12-23 : 12 bits */
	uint16_t table_id_extension;			/* 24-39 : 16 bits */
	// reserved								/* 40-41 :  2 bits */
	uint8_t version_number;					/* 42-46 :  5 bits */
	uint8_t current_next_indicator;			/* 47-47 :  1 bit  */
	uint8_t section_number;					/* 48-55 :  8 bits */
	uint8_t last_section_number;			/* 56-63 :  8 bits */
} ST_SECTION_HEADER;


#endif
