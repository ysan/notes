#!/bin/bash

set -eu

DL_URL_PORTABLE_ANSIBLE=https://github.com/ownport/portable-ansible/releases/download/v0.5.0/portable-ansible-v0.5.0-py3.tar.bz2
DL_URL_ANSIBLE_MODULE_POSIX=https://github.com/ansible-collections/ansible.posix/archive/refs/tags/1.2.0.tar.gz
DL_URL_SSHPASS=https://sourceforge.net/projects/sshpass/files/sshpass/1.06/sshpass-1.06.tar.gz


function _run () {
	mkdir -p ansible
	cd ansible

	if [ ! -f "$(basename ${DL_URL_PORTABLE_ANSIBLE})" ]; then
		curl --insecure -L -O "${DL_URL_PORTABLE_ANSIBLE}"
	fi
	if [ -f "$(basename ${DL_URL_PORTABLE_ANSIBLE})" ]; then
		if [ ! -d ./ansible ]; then
			tar xvf "$(basename ${DL_URL_PORTABLE_ANSIBLE})"
		fi
	fi

	if [ -d ./ansible ]; then
		for ITEM in config console doc galaxy inventory playbook pull vault; do
			if [ ! -L ./ansible-${ITEM} ]; then
				ln -s ansible ansible-${ITEM}
			fi
		done
	fi

	mkdir -p modules
	cd modules

	if [ ! -f "$(basename ${DL_URL_ANSIBLE_MODULE_POSIX})" ]; then
		curl --insecure -L -O "${DL_URL_ANSIBLE_MODULE_POSIX}"
	fi
	if [ -f "$(basename ${DL_URL_ANSIBLE_MODULE_POSIX})" ]; then
		if [ ! -d ./ansible.posix-1.2.0 ]; then
			tar xvf "$(basename ${DL_URL_ANSIBLE_MODULE_POSIX})"
		fi
	fi

	cd ../../


	if [ ! -f "$(basename ${DL_URL_SSHPASS})" ]; then
		curl --insecure -L -O "${DL_URL_SSHPASS}"
	fi
	if [ -f "$(basename ${DL_URL_SSHPASS})" ]; then
		if [ ! -d ./sshpass-1.06 ]; then
			tar xvf "$(basename ${DL_URL_SSHPASS})"
			cd sshpass-1.06
			./configure
			make >/dev/null 2>&1
			cd ../
		fi

		mkdir -p roles/sshpass/files
		cp -p "$(basename ${DL_URL_SSHPASS})" roles/sshpass/files
	fi

	echo
	echo
	echo "############################"
	echo "  setup ansible completed."
	echo "############################"
}

function _clean () {
	rm -rf ansible
	rm -rf "$(basename ${DL_URL_SSHPASS})"
	rm -rf sshpass-1.06
#	rm -rf "files/$(basename ${DL_URL_SSHPASS})"
	rm -rf "roles/sshpass/files/$(basename ${DL_URL_SSHPASS})"
}

function _usage () {
	echo
	echo "Usage: $0 run|clean"
	echo
}


if [ $# -ne 1 ]; then
	_usage
else
	case $1 in
		"run")
			_run
			;;
		"clean")
			_clean
			;;
		*)
			echo "invalid arguments :[$*]"
			_usage
			;;
	esac
fi
