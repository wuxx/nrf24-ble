#!/bin/bash
BADUSB_BIN=$1
if [ ${#BADUSB_BIN} -eq 0 ]; then
	echo "$0 bin"
	exit 1
fi

./pad.exe ${BADUSB_BIN} 8192
./build_cmd.exe $1 0x20002000 0x0800e000 > badusb.txt
