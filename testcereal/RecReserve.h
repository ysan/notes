#ifndef _REC_MANAGER_H_
#define _REC_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Etime.h"

#include "cereal/cereal.hpp"


typedef enum {
	EN_RESERVE_STATE__INIT = 0,
	EN_RESERVE_STATE__REQ_START_RECORDING,
	EN_RESERVE_STATE__NOW_RECORDING,
	EN_RESERVE_STATE__END_SUCCESS,
	EN_RESERVE_STATE__END_ERROR__FORCE_STOP,
	EN_RESERVE_STATE__END_ERROR__ALREADY_PASSED,
	EN_RESERVE_STATE__END_ERROR__HDD_FREE_SPACE_LOW,
	EN_RESERVE_STATE__END_ERROR__INTERNAL_ERR,
} EN_RESERVE_STATE;

const char *g_reserveState [] = {
	"INIT",
	"REQ_START_RECORDING",
	"NOW_RECORDING",
	"END_SUCCESS",
	"END_ERROR__FORCE_STOP",
	"END_ERROR__ALREADY_PASSED",
	"END_ERROR__HDD_FREE_SPACE_LOW",
	"END_ERROR__INTERNAL_ERR",
};


class CRecReserve {
public:
	CRecReserve (void) {
		clear ();
	}
	~CRecReserve (void) {
		clear ();
	}

	bool operator == (const CRecReserve &obj) const {
		if (
			this->transport_stream_id == obj.transport_stream_id &&
			this->original_network_id == obj.original_network_id &&
			this->service_id == obj.service_id &&
			this->event_id == obj.event_id &&
			this->start_time == obj.start_time &&
			this->end_time == obj.end_time
		) {
			return true;
		} else {
			return false;
		}
	}

	bool operator != (const CRecReserve &obj) const {
		if (
			this->transport_stream_id != obj.transport_stream_id ||
			this->original_network_id != obj.original_network_id ||
			this->service_id != obj.service_id ||
			this->event_id != obj.event_id ||
			this->start_time != obj.start_time ||
			this->end_time != obj.end_time
		) {
			return true;
		} else {
			return false;
		}
	}

	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;

	uint16_t event_id;
	CEtime start_time;
	CEtime end_time;

	std::string title_name;

	EN_RESERVE_STATE state;
	bool is_used;


	void set (
		uint16_t _transport_stream_id,
		uint16_t _original_network_id,
		uint16_t _service_id,
		uint16_t _event_id,
		CEtime* p_start_time,
		CEtime* p_end_time,
		std::string _title_name
	)
	{
		this->transport_stream_id = _transport_stream_id;
		this->original_network_id = _original_network_id;
		this->service_id = _service_id;
		this->event_id = _event_id;
		this->start_time = *p_start_time;
		this->end_time = *p_end_time;
		this->title_name = _title_name;
		this->state = EN_RESERVE_STATE__INIT;
		this->is_used = true;
	}

	void clear (void) {
//TODO 適当クリア
		// clear all
		memset (this, 0x00, sizeof(CRecReserve));
		start_time.clear();
		end_time.clear();
		state = EN_RESERVE_STATE__INIT;
		is_used = false;
	}

	void dump (void) {
		printf (
			"tsid:[0x%04x] org_nid:[0x%04x] svcid:[0x%04x] evtid:[0x%04x]\n",
			transport_stream_id,
			original_network_id,
			service_id,
			event_id
		);
		printf (
			"time:[%s - %s] state:[%s]\n",
			start_time.toString(),
			end_time.toString(),
			g_reserveState [state]
		);
		printf ("title:[%s]\n", title_name.c_str());
	}

};



#endif
