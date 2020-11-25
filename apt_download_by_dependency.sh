#
# 対象debパッケージのダウンロード 依存するパッケージを再起的にダウンロードする
# TODO apt-cache dependsの結果 Dpends: <xxx> となっている場合について対応できていない
#
# あとから以下コマンドを行うのと同じことを知った..
# sudo apt clean
# sudo apt --download-only install パッケージ名
# ls -ltr /var/cache/apt/archives/
#

#!/bin/bash

if [ $# -ne 1 ]; then
	echo "usage: $0 package-name"
	exit 1
fi

LANG=C

# arg $1 - package-name
# arg $2 - depth
function download () {
	local TAB=""
	for (( i = 0; i < $2; ++i ))
	do
	    TAB="${TAB}  "
	done

	apt-get download $1 2>&1 | awk -v tab="${TAB}" '{printf "%s%s\n", tab, $0}'
#	if [ $? -eq 0 ]; then
	if [ ${PIPESTATUS[0]} -eq 0 ]; then
		echo "${TAB}download:[$1] success."
#		return 0
	else
		echo "${TAB}download:[$1] error !!"
		echo
#		return 1
	fi

	echo "${TAB}check dependency:[$1]"
	apt-cache depends $1 2>&1 | awk -v tab="${TAB}" '{printf "%s%s\n", tab, $0}'
#	if [ ! $? -eq 0 ]; then
	if [ ! ${PIPESTATUS[0]} -eq 0 ]; then
		echo "${TAB}check dependency:[$1] error !!"
		echo
		return 1
	fi

	apt-cache depends $1 | grep Depends | grep -v "PreDepends" | awk '{print $2}' > packagelist.$$.$2  2>/dev/null
#	if [ $? -eq 0 ]; then
	if [ ${PIPESTATUS[0]} -eq 0 ]; then
		while read PACKAGE
		do
#			echo "line:[${PACKAGE}]"
			if [ ${#PACKAGE} -gt 0 ]; then
				RSLT=`echo $1 | dpkg -l | awk '{print $2}' | grep ${PACKAGE}`
				if [ -z "${RSLT}" ]; then
					# not yet installed
					echo -n "${TAB}check package:[$PACKAGE]"

					# check duplicate
					grep ${PACKAGE} packagelist.$$.total > /dev/null 2>&1
					if [ ! $? -eq 0 ]; then
						echo " --> do download"

						echo ${PACKAGE} >> packagelist.$$.total
						download ${PACKAGE} `expr $2 + 1`
						if [ ! $? -eq 0 ]; then
							# error handle
#							return 1
							continue
						fi
					else
						# duplicated package -> ignore
						echo " --> ignore"
						continue
					fi

				else
					# already installed
					echo "${TAB}already installed:[$PACKAGE]"
				fi
			fi
		done <<- END
		`cat packagelist.$$.$2`
		END
	fi
	rm -f packagelist.$$.$2

}


echo "$0 begin"
touch packagelist.$$.total

download $1 0
if [ $? -eq 0 ]; then
	rm -f packagelist.$$.total
	echo "$0 success end"
	exit 0;
else 
#	rm -f packagelist.$$.total
	echo "$0 failure end !!"
	exit 1;
fi
