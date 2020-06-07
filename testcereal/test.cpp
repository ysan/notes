#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/memory.hpp"

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

	// array
//	CRecReserve r[3];
//	r[0].transport_stream_id = 1;
//	r[0].original_network_id = 1;
//	r[0].service_id = 1;
//	r[0].event_id = 1;
//	r[0].start_time.setCurrentTime();
//	r[0].end_time.setCurrentTime();
//	r[0].title_name = "テスト1";
//	r[0].state = EN_RESERVE_STATE__NOW_RECORDING;
//	r[0].is_used = true;

//	r[1].transport_stream_id = 2;
//	r[1].original_network_id = 2;
//	r[1].service_id = 2;
//	r[1].event_id = 2;
//	r[1].start_time.setCurrentTime();
//	r[1].end_time.setCurrentTime();
//	r[1].title_name = "テスト2";
//	r[1].state = EN_RESERVE_STATE__NOW_RECORDING;
//	r[1].is_used = true;

//	r[2].transport_stream_id = 3;
//	r[2].original_network_id = 3;
//	r[2].service_id = 3;
//	r[2].event_id = 3;
//	r[2].start_time.setCurrentTime();
//	r[2].end_time.setCurrentTime();
//	r[2].title_name = "テスト3";
//	r[2].state = EN_RESERVE_STATE__NOW_RECORDING;
//	r[2].is_used = true;

	// vector
//	std::vector<CRecReserve> resvs;
//	resvs.push_back(r[0]);
//	resvs.push_back(r[1]);
//	resvs.push_back(r[2]);

	// uniq_ptr vector
	std::vector<std::unique_ptr<CRecReserve>> resvs_uptr;
	for (int i = 0; i < 3; ++ i) {
		CRecReserve *p = new CRecReserve();
		p->transport_stream_id = i+1;
		p->original_network_id = i+1;
		p->service_id = i+1;
		p->event_id = i+1;
		p->start_time.setCurrentTime();
		p->end_time.setCurrentTime();
		std::stringstream ss;
		ss << "テスト" << i+1;
		p->title_name = ss.str();
		p->state = EN_RESERVE_STATE__NOW_RECORDING;
		p->is_used = true;
		std::unique_ptr<CRecReserve> r_uptr (p);
		resvs_uptr.push_back (std::move(r_uptr));
	}

	std::stringstream ss;
	{
		cereal::JSONOutputArchive o_archive (ss);

//		o_archive (r, sizeof(CRecReserve) * 3); // array
//		o_archive (cereal::make_nvp("r", resvs)); // vector
		o_archive(cereal::make_nvp("r", resvs_uptr)); // uniq_ptr vector
	}

	std::ofstream ofs (path, std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();

	// ----------------------------------

	std::stringstream ss2;
	std::ifstream ifs (path, std::ios::in);
	ss2 << ifs.rdbuf();

//	CRecReserve rr[3]; // array
//	std::vector<CRecReserve> rresvs; // vector
	std::vector<std::unique_ptr<CRecReserve>> rresvs_uptr; // uniq_ptr vector

	cereal::JSONInputArchive i_archive (ss2);
//	i_archive (rr, sizeof(CRecReserve)*3); // array
//	i_archive (cereal::make_nvp("r", rresvs)); // vector
	i_archive(cereal::make_nvp("r", rresvs_uptr)); // uniq_ptr vector

	ifs.close();
	ss2.clear();



	//--- result ---

	// array
//	rr[0].dump();
//	rr[1].dump();
//	rr[2].dump();

	// vector
//	for (auto &r : rresvs) {
//		r.dump();
//	}

	// uniq_ptr vector
	for (auto &r : rresvs_uptr) {
		r.get()->dump();
	}


	exit (EXIT_SUCCESS);
}
