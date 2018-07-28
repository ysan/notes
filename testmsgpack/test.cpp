#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>

#include "msgpack.hpp" 
 

using namespace std;
//using namespace msgpack;


int main (void)
{
	//-------------------------    single    -------------------------//
	{
		//===== serialize =====
		vector<char> vec;
		vec.push_back('a');
		vec.push_back('b');
		vec.push_back('c');

		msgpack::sbuffer buff;
		msgpack::packer<msgpack::sbuffer> pk (&buff);

		pk.pack (vec);


		// ----------
		char *ppack = buff.data();
		int packsize = buff.size();
		// ----------


		//===== deserialize =====
		vector<char> upk_vec;

		msgpack::v1::unpacked upk;
		msgpack::v1::unpack (&upk, ppack, packsize);

		msgpack::object obj = upk.get();
		obj.convert (upk_vec);



		// result
		for (vector<char>::iterator iter = upk_vec.begin(); iter != upk_vec.end(); iter ++) {
			printf ("deserialized vec [%c]\n", *iter);
		}


	}



	//-------------------------    structure    -------------------------//
	{
		//===== serialize =====
		int int_val = 10000;
		short short_val = 540;
		char char_array[6] = {'a','b','c','d','e','\0'};
		vector<int> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		map <int, int> vmap = {
			{1, 100001},
			{2, 100002},
			{3, 100003},
		};


		msgpack::sbuffer buff;
		msgpack::packer<msgpack::sbuffer> pk (&buff);

		pk.pack_array (5);

		// int
		pk.pack (int_val);
		// short
		pk.pack (short_val);
		// char array
		pk.pack_bin (sizeof(char_array));
		pk.pack_bin_body (char_array, sizeof(char_array));
		// vector
		pk.pack (vec);
		// map
		pk.pack (vmap);


		// ----------
		char *ppack = buff.data();
		int packsize = buff.size();
		// ----------


		//===== deserialize =====
		int upk_int_val = 0;
		short upk_short_val = 0;
		char upk_char_array[6] = {0};
		vector<int> upk_vec;
		map <int, int> upk_map;


		msgpack::v1::unpacked upk;
		msgpack::v1::unpack (&upk, ppack, packsize);

		msgpack::object obj = upk.get();
		msgpack::object_array obj_array = obj.via.array;

		// int
		(obj_array.ptr[0]).convert (upk_int_val);
		// short
		(obj_array.ptr[1]).convert (upk_short_val);
		// char array
		msgpack::object_bin obj_bin = (obj_array.ptr[2]).via.bin;
		strncpy (upk_char_array, (char*)obj_bin.ptr, obj_bin.size);
		// vector
		obj_array.ptr[3].convert (upk_vec);
		// map
		obj_array.ptr[4].convert (upk_map);



		// result
		printf ("deserialized int [%d]\n", upk_int_val);
		printf ("deserialized short [%d]\n", upk_short_val);
		printf ("deserialized char array [%s](size%d)\n", upk_char_array, obj_bin.size);
		vector<int>::iterator iter_vec = upk_vec.begin();
		while (iter_vec != upk_vec.end()) {
			printf ("deserialized vec [%d]\n", *iter_vec);
			iter_vec ++;
		}
		map<int, int>::iterator iter_map = upk_map.begin();
		while (iter_map != upk_map.end()) {
			printf ("deserialized map [%d, %d]\n", iter_map->first, iter_map->second);
			iter_map ++;
		}

	}


	//-------------------------    tuple    -------------------------//
	{
		//===== serialize =====
		int int_val = 10000;
		short short_val = 540;
		vector<int> vec;
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		map <int, int> vmap = {
			{1, 100001},
			{2, 100002},
			{3, 100003},
		};

		msgpack::type::tuple<int, short, vector<int>, map <int, int>> src(int_val, short_val, vec, vmap);

		msgpack::sbuffer buff;
		msgpack::packer<msgpack::sbuffer> pk (&buff);

		pk.pack (src);


		// ----------
		char *ppack = buff.data();
		int packsize = buff.size();
		// ----------


		//===== deserialize =====
		int upk_int_val = 0;
		short upk_short_val = 0;
		vector<int> upk_vec;
		map <int, int> upk_map;


		msgpack::object_handle oh = msgpack::unpack (ppack, packsize);
		msgpack::object obj = oh.get();

		msgpack::type::tuple<int, short, vector<int>, map <int, int>> dst;
		obj.convert(dst);
//		std::cout << obj << std::endl;


		upk_int_val = dst.get<0>();
		upk_short_val = dst.get<1>();
		upk_vec = dst.get<2>();
		upk_map = dst.get<3>();

		// result
		printf ("deserialized int [%d]\n", upk_int_val);
		printf ("deserialized short [%d]\n", upk_short_val);
		for (vector<int>::iterator iter_vec = upk_vec.begin(); iter_vec != upk_vec.end(); iter_vec ++) {
			printf ("deserialized vec [%d]\n", *iter_vec);
		}
		for (map<int, int>::iterator iter_map = upk_map.begin(); iter_map != upk_map.end(); iter_map ++) {
			printf ("deserialized map [%d, %d]\n", iter_map->first, iter_map->second);
		}


	}


	exit (EXIT_SUCCESS);
}
