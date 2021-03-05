#!/bin/bash

VIDEO_ID=""
OUT_DIR=""

if [ $# -eq 1 ]; then
	VIDEO_ID=$1
	OUT_DIR="./"
elif [ $# -eq 2 ]; then
	VIDEO_ID=$1
	OUT_DIR=$2
else
	echo "Usage: $0 video_id [output_directory]";
	exit 1
fi

if [ ! -d ${OUT_DIR} ]; then
	echo "not existed output directory... [${OUT_DIR}]"
	exit 1
fi

echo "video_id:[${VIDEO_ID}]"
echo "output_directory:[${OUT_DIR}]"

RES=`curl -s -A "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.0 Safari/605.1.15" https://www.youtube.com/get_video_info?video_id=${VIDEO_ID}`

DECODED=`echo ${RES} | grep -o -E "manifest\.googlevideo\.com.+\.m3u8" | nkf --url-input`

URL="https://${DECODED}"
echo "url:[${URL}]"

ffmpeg -i "${URL}" -c copy ${OUT_DIR}/${VIDEO_ID}.ts
