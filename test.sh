#!/usr/bin/env bash
# Author: Dominik Harmim <harmim6@gmail.com>

OUT='ots'
SOURCE="$OUT.cpp"
BUILD_OPTIONS='-std=c++14'
#BUILD_OPTIONS='-std=c++14 -D DEBUG'
RUN_OPTIONS='-q'
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
	RUN_OPTIONS="$RUN_OPTIONS --oversubscribe"
else
	BUILD_OPTIONS="$BUILD_OPTIONS --prefix /usr/local/share/OpenMPI"
	RUN_OPTIONS="$RUN_OPTIONS --prefix /usr/local/share/OpenMPI"
fi

mpic++ ${BUILD_OPTIONS} -o "$OUT" "$SOURCE"
CODE="$?"
if [[ "$CODE" -ne 0 ]]; then
	exit "$CODE"
fi

DD_OUT="$(dd if=/dev/random bs=1 count="$NUMBERS" of="$NUMBERS_FILE" 2>&1)"
CODE="$?"
if [[ "$CODE" -ne 0 ]]; then
	echo "$DD_OUT"
	rm -f "$OUT"
	exit "$CODE"
fi

mpirun ${RUN_OPTIONS} -np "$NUMBERS" "$OUT"
CODE="$?"
if [[ "$CODE" -ne 0 ]]; then
	rm -f "$OUT" "$NUMBERS_FILE"
	exit "$CODE"
fi

rm -f "$OUT" "$NUMBERS_FILE"
