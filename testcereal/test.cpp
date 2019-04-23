#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <string>

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"

#include "RecReserve.h"


template <class Archive>
void serialize (Archive &archive, struct timespec &t)
{
	archive (cereal::make_nvp("tv_sec", t.tv_sec), cereal::make_nvp("tv_nsec", t.tv_nsec));
}

template <class Archive>
void serialize (Archive &archive, CEtime &t)
{
	archive (cereal::make_nvp("m_time", t.m_time));
}

template <class Archive>
void serialize (Archive &archive, CRecReserve &r)
{
	archive (
		cereal::make_nvp("transport_stream_id", r.transport_stream_id),
		cereal::make_nvp("original_network_id", r.original_network_id),
		cereal::make_nvp("service_id", r.service_id),
		cereal::make_nvp("event_id", r.event_id),
		cereal::make_nvp("start_time", r.start_time),
		cereal::make_nvp("end_time", r.end_time),
		cereal::make_nvp("title_name", r.title_name),
		cereal::make_nvp("state", r.state),
		cereal::make_nvp("is_used", r.is_used)
	);
}


int main (void)
{
	std::string path = "./tmp.json";


	CRecReserve r[3];
	r[0].transport_stream_id = 1;
	r[0].original_network_id = 1;
	r[0].service_id = 1;
	r[0].event_id = 1;
	r[0].start_time.setCurrentTime();
	r[0].end_time.setCurrentTime();
	r[0].title_name = "テスト";
	r[0].state = EN_RESERVE_STATE__NOW_RECORDING;
	r[0].is_used = true;

	r[1].transport_stream_id = 2;
	r[1].original_network_id = 2;
	r[1].service_id = 2;
	r[1].event_id = 2;
	r[1].start_time.setCurrentTime();
	r[1].end_time.setCurrentTime();
	r[1].title_name = "テスト2";
	r[1].state = EN_RESERVE_STATE__NOW_RECORDING;
	r[1].is_used = true;

	r[2].transport_stream_id = 3;
	r[2].original_network_id = 3;
	r[2].service_id = 3;
	r[2].event_id = 3;
	r[2].start_time.setCurrentTime();
	r[2].end_time.setCurrentTime();
	r[2].title_name = "テスト3";
	r[2].state = EN_RESERVE_STATE__NOW_RECORDING;
	r[2].is_used = true;


	std::stringstream ss;
	{
		cereal::JSONOutputArchive o_archive (ss);
		o_archive (r, sizeof(CRecReserve) * 3);
    }

	std::ofstream ofs (path, std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();

	// ----------------------------------

	std::stringstream ss2;
	std::ifstream ifs (path, std::ios::in);
	ss2 << ifs.rdbuf();

	CRecReserve rr[3];
	cereal::JSONInputArchive i_archive (ss2);
	i_archive (rr, sizeof(CRecReserve)*3);

	ifs.close();
	ss2.clear();



	// result
	rr[0].dump();
	rr[1].dump();
	rr[2].dump();


	exit (EXIT_SUCCESS);
}
