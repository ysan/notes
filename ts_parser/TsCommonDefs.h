#ifndef _TS_COMMON_DEF_H_
#define _TS_COMMON_DEF_H_


#define TS_PACKET_LEN					(188)
#define SYNC_BYTE						(0x47)
#define TS_HEADER_LEN					(4)
#define SECTION_HEADER_LEN				(8)
#define SECTION_HEADER_FIX_LEN			(5) // after section_length
#define SECTION_SHORT_HEADER_LEN		(3) // section_length include, before
#define SECTION_CRC32_LEN				(4)

#define MAXSECLEN						(4096)

// pid
#define PID_PAT							(0x0000)
#define PID_CAT							(0x0001)
#define PID_TSDT						(0x0002)
#define PID_NIT							(0x0010)
#define PID_SDT							(0x0011)
#define PID_BAT							(0x0011)
#define PID_EIT_H						(0x0012)
#define PID_RST							(0x0013)
#define PID_TOT							(0x0014)
#define PID_TDT							(0x0014)
#define PID_DCT							(0x0017)
#define PID_DIT							(0x001e)
#define PID_SIT							(0x001f)
#define PID_PCAT						(0x0022)
#define PID_SDTT						(0x0023)
#define PID_W_SDTT						(0x0023)
#define PID_BIT							(0x0024)
#define PID_NBIT						(0x0025)
#define PID_LDT							(0x0025)
#define PID_EIT_M						(0x0026)
#define PID_EIT_L						(0x0027)
#define PID_S_SDTT						(0x0028)
#define PID_CDT							(0x0029)
#define PID_ETT							(0x0030)
#define PID_EMT							(0x0030)
#define PID_SLDT						(0x0036)
#define PID_NULL						(0x1fff)

// table id
#define TBL_ID__PAT						(0x00)
#define TBL_ID__CAT						(0x01)
#define TBL_ID__PMT						(0x02)
#define TBL_ID__TSDT					(0x03)
#define TBL_ID__NIT_A					(0x40) // actual
#define TBL_ID__NIT_O					(0x41) // other
#define TBL_ID__SDT_A					(0x42) // actual
#define TBL_ID__SDT_O					(0x46) // other
#define TBL_ID__BAT						(0x4a)
#define TBL_ID__EIT_PF_A				(0x4e) // actual
#define TBL_ID__EIT_PF_O				(0x4f) // other
#define TBL_ID__EIT_SCH_A				(0x50) // actual
#define TBL_ID__EIT_SCH_A_EXT			(0x58) // actual extend
#define TBL_ID__EIT_SCH_O				(0x60) // other
#define TBL_ID__EIT_SCH_O_EXT			(0x68) // other extend
#define TBL_ID__TDT						(0x70)
#define TBL_ID__RST						(0x71)
#define TBL_ID__ST						(0x72)
#define TBL_ID__TOT						(0x73)
#define TBL_ID__DIT						(0x7e)
#define TBL_ID__SIT						(0x7f)
#define TBL_ID__ECM						(0x82)
#define TBL_ID__EMM_NARROW				(0x84)
#define TBL_ID__EMM_INDV_CMN			(0x85)
#define TBL_ID__SLDT_A					(0xb2) // actual
#define TBL_ID__SLDT_O					(0xb6) // other
#define TBL_ID__ETT_A					(0xa2) // actual
#define TBL_ID__ETT_O					(0xa3) // other
#define TBL_ID__EMT_G					(0xa4)
#define TBL_ID__EMT_N					(0xa5)
#define TBL_ID__EMT_D					(0xa7)
#define TBL_ID__SDTT					(0xc3)
#define TBL_ID__BIT						(0xc4)
#define TBL_ID__NBIT_MSG				(0xc5)
#define TBL_ID__NBIT_REF				(0xc6)
#define TBL_ID__LDT						(0xc7)
#define TBL_ID__CDT						(0xc8)



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
	uint8_t sync;							//  0- 7 :  8 bits
	uint8_t transport_error_indicator;		//  8- 8 :  1 bit
	uint8_t payload_unit_start_indicator;	//  9- 9 :  1 bit
	uint8_t transport_priority;				// 10-10 :  1 bits
	uint16_t pid;							// 11-23 : 13 bits
	uint8_t transport_scrambling_control;	// 24-25 :  2 bits
	uint8_t adaptation_field_control;		// 26-27 :  2 bits
	uint8_t continuity_counter;				// 28-31 :  4 bits
} ST_TS_HEADER;


typedef struct {
	uint8_t table_id;						//  0- 7 :  8 bits
	uint8_t section_syntax_indicator;		//  8- 8 :  1 bit
	uint8_t private_indicator;				//  9- 9 :  1 bit
	// reserved								// 10-11 :  2 bits
	uint16_t section_length;				// 12-23 : 12 bits
	uint16_t table_id_extension;			// 24-39 : 16 bits
	// reserved								// 40-41 :  2 bits
	uint8_t version_number;					// 42-46 :  5 bits
	uint8_t current_next_indicator;			// 47-47 :  1 bit
	uint8_t section_number;					// 48-55 :  8 bits
	uint8_t last_section_number;			// 56-63 :  8 bits
} ST_SECTION_HEADER;


#endif
