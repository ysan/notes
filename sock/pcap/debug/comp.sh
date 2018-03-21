#!/bin/sh

cc -g -Wall tmp.c ../pcap_addr2name.c -I../ -I../../../mycommon/ -L../../../mycommon/ -lmycommon
