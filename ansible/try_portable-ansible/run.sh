#!/bin/bash

#set -eu

TARGET_HOST=""

if [ $# -eq 1 ]; then
	TARGET_HOST=$1
elif [ $# -gt 1 ] || [ $# -eq 0 ]; then
	echo
	echo "usage: $0 targethost"
	echo "  targethost: localhost or dev_{NO} or dev_all"
	echo
	exit 1
fi
echo "TARGET_HOST=${TARGET_HOST}"


export ANSIBLE_LIBRARY=./ansible/modules/ansible.posix-1.2.0
export ANSIBLE_LOG_PATH=./log/run_$(date +"%Y%m%d%H%M%S").log

# check portable-ansible and sshpass
python3 ./ansible/ansible-playbook --version
if [ $? -ne 0 ]; then
	echo
	echo "Meybe ansible not installed. Please run \"./setup_portable-ansible.sh\""
	exit 1
fi
./sshpass-1.06/sshpass -V | grep sshpass
if [ $? -ne 0 ]; then
	echo
	echo "Meybe sshpass not installed. Please run \"./setup_portable-ansible.sh\""
	exit 1
fi


export PATH=./sshpass-1.06:${PATH}

TARGET_USER=pi
TARGET_PASS=raspberry
SUDO_USER=pi
SUDO_PASS=raspberry

python3 ./ansible/ansible-playbook -i hosts.yml -l ${TARGET_HOST} main.yml \
	--extra-vars="target_user=${TARGET_USER} target_pass=${TARGET_PASS}" \
	--extra-vars="sudo_user=${SUDO_USER} sudo_pass=${SUDO_PASS}"
