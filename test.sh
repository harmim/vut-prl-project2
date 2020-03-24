#!/usr/bin/env bash

# Author: Dominik Harmim <harmim6@gmail.com>

OUT='ots'
SOURCE="$OUT.cpp"
BUILD_OPTIONS=''
RUN_OPTIONS=''
NUMBERS_FILE='numbers'

if [[ "$#" -ne 1 ]]; then
	echo "Expecting one argument: $0 numbers-count" >&2
	exit 1
else
	NUMBERS="$1"
fi

if [[ `uname -s` = "Darwin" ]]; then
	export OMPI_MCA_btl='self,tcp'
	export PMIX_MCA_gds='^ds12'
	RUN_OPTIONS='--oversubscribe'
else
	BUILD_OPTIONS='--prefix /usr/local/share/OpenMPI'
	RUN_OPTIONS='--prefix /usr/local/share/OpenMPI'
fi

mpic++ "$BUILD_OPTIONS" -o "$OUT" "$SOURCE"
dd if=/dev/random bs=1 count="$NUMBERS" of="$NUMBERS_FILE" > /dev/null 2>&1
mpirun "$RUN_OPTIONS" -np "$NUMBERS" "$OUT"
rm -f "$OUT" "$NUMBERS_FILE"
