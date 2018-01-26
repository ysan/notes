#!/bin/sh

ARG_BRANCH_LIST=()
ARG_BRANCH_NUM=0
IS_CHECK_ONLY=1
IS_DIFF_REMOTE=0


while getopts b:ed OPT
do
	case ${OPT} in
	'b') ARG_BRANCH_LIST+=(${OPTARG})
		echo "ARG_BRANCH_LIST+=${OPTARG}"
		ARG_BRANCH_NUM=`expr ${ARG_BRANCH_NUM} + 1`
		;;
	'e') IS_CHECK_ONLY=0
		echo "exec fetch and merge"
		;;
	'd') IS_DIFF_REMOTE=1
		echo "add proc (git diff remote)"
		;;
	esac
done


REMOTE_BRANCH_LIST=()
REMOTE_BRANCH_NUM=0
for REMOTE_BRANCH_NAME in `git branch -r | grep -v HEAD | sed -E 's/^.+origin\///g'`
do
	REMOTE_BRANCH_NUM=`expr ${REMOTE_BRANCH_NUM} + 1`
	if [ ${IS_DIFF_REMOTE} -eq 1 ]; then
		git checkout ${REMOTE_BRANCH_NAME} >/dev/null 2>&1

#		git fetch origin ${REMOTE_BRANCH_NAME}
#		RSLT=`git diff ${REMOTE_BRANCH_NAME}..origin/${REMOTE_BRANCH_NAME} | grep -e "^diff" | wc -l`
#		if [ ${RSLT} -gt 0 ]; then
#			printf "+[%3d diffs]  " ${RSLT}
#		else
#			printf " [  none   ]  "
#		fi

		REMOTE_HEAD=`git ls-remote origin ${REMOTE_BRANCH_NAME} | awk '{printf $1}'`
		LP=0
		IS_FOUND=0
		while read LINE
		do
			if [ ${LINE} = ${REMOTE_HEAD} ]; then
#				echo "equal ${REMOTE_HEAD}"
				IS_FOUND=1
				break
			fi
			LP=`expr ${N} + 1`
		done <<- END
		`git log | grep -E "^commit" | sed -E 's/commit +//g'`
		END

		if [ ${IS_FOUND} -eq 1 ]; then
			if [ ${LP} -eq 0 ]; then
				# no difference
				printf "[no difference from remote] "
			else
				# local commit is ahead
				printf "[local commit is ahead]     "
			fi
		else
			# remote commit is ahead
			printf     "[remote commit is ahead]    "
		fi

	fi
	echo "${REMOTE_BRANCH_NUM}: ${REMOTE_BRANCH_NAME}"
	REMOTE_BRANCH_LIST+=(${REMOTE_BRANCH_NAME})
done

echo "total branch num: [${REMOTE_BRANCH_NUM}]"
echo ""


BRANCH_LIST=()
BRANCH_NUM=0
IS_FOUND=0
if  [ ${ARG_BRANCH_NUM} -gt 0 ]; then
	echo "check argument target branch name"
	for TMP in ${ARG_BRANCH_LIST[@]}
	do
		for RTMP in ${REMOTE_BRANCH_LIST[@]}
		do
			if [ ${RTMP} = ${TMP} ]; then
				BRANCH_LIST+=(${TMP})
				BRANCH_NUM=`expr ${BRANCH_NUM} + 1`
				IS_FOUND=1
				break;
			fi
		done

		if [ ${IS_FOUND} -eq 0 ]; then
			echo "  [${TMP}] is invalid branch name !!!  --> skip"
		else
			echo "  [${TMP}] is valid."
		fi
		IS_FOUND=0
	done
else
	echo "target is all branch."
	for TMP in ${REMOTE_BRANCH_LIST[@]}
	do
		BRANCH_LIST+=(${TMP})
	done
	BRANCH_NUM=${REMOTE_BRANCH_NUM}
fi
echo ""



if [ ${IS_CHECK_ONLY} -eq 1 ]; then
	exit 1
fi


N=0
#for TMP in `git branch -r | grep -v HEAD | sed -E 's/^.+origin\///g'`
for TMP in ${BRANCH_LIST[@]}
do
	N=`expr ${N} + 1`
	echo ""
	echo "[${N}]: ===== branch [${TMP}] ====="
	echo ""
	git checkout ${TMP}
#	git pull --rebase origin ${TMP}
	git fetch origin ${TMP}
	git merge origin/${TMP}
done

