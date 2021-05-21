#!/bin/bash


SESSION_NAME=""
tmux display-message -p "#S" > /dev/null 2>&1
if [ $? -eq 0 ]; then
	SESSION_NAME=`tmux display-message -p "#S"`
#	WINDOW_NAME=`tmux display-message -p '#I'`
else
	SESSION_NAME=4x3_tiled_ssh
fi
WINDOW_NAME=4x3_tiled_ssh


if [ $# -eq 1 ]; then
	if [ $1 == "-k" ]; then
		tmux kill-session -t ${SESSION_NAME}
		if [ $? -eq 0 ]; then
			echo "session killed. session:[${SESSION_NAME}]"
			exit 0
		else
			exit 1
		fi
	else
		echo "usage: $0 [-k]"
		echo "  -k: kill session"
		exit 1
	fi
elif [ $# -gt 1 ]; then
	echo "usage: $0 [-k]"
	echo "  -k: kill session"
	exit 1
fi


if [ ${#SSH_USER} -eq 0 ]; then
	echo "Please set \"export SSH_USER=<your ssh user>\"."
	exit 1
fi
if [ ${#SSH_PASS} -eq 0 ]; then
	echo "Please set \"export SSH_PASS=<your ssh password>\"."
	exit 1
fi


if [ ${SESSION_NAME} == 4x3_tiled_ssh ]; then
	echo "create new session."
	tmux new-session -s ${SESSION_NAME} -d
fi
tmux new-window -n ${WINDOW_NAME} -t ${SESSION_NAME}:


for ((i = 0; i < 12 -1; i ++)); do
	tmux split-window -v -t ${SESSION_NAME}:${WINDOW_NAME}.0
	tmux select-layout tiled
done

tmux select-pane -t ${SESSION_NAME}:${WINDOW_NAME}.0
tmux select-layout tiled

for ((i = 0; i < 12; i ++)); do
	N=`expr ${i} + 1`
	NUM=`printf %02d ${N}`
#	tmux send-keys -t ${i} "ssh ${SSH_USER}@192.168.100.1${NUM}" C-m
	tmux send-keys -t ${SESSION_NAME}:${WINDOW_NAME}.${i} "ssh ${SSH_USER}@localhost" C-m  # local-debug
	sleep 2
	tmux send-keys -t ${SESSION_NAME}:${WINDOW_NAME}.${i} "${SSH_PASS}" C-m
	echo "pane ${SESSION_NAME}:${WINDOW_NAME}.${i} completed."
done

tmux set-window-option -t ${SESSION_NAME}: synchronize-panes on

if [ ${SESSION_NAME} == 4x3_tiled_ssh ]; then
	tmux a -t ${SESSION_NAME}:
fi
