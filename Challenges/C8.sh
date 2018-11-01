#!/bin/bash

N=${1:-100}

echo "Ustvarjam $N procesov..."
echo -n "PIDS: "

for ((i=0; i<$N; i++))
do
	if (($RANDOM % 3 == 0))
	then
		xclock &
	else
		xeyes &
	fi

	PID[i]="$!"
	echo -n "$! "
done

echo ""

read -s -n 1

echo "Ukinjam procese..."
echo -n "KILLS: "

for ((i=$(($N - 1)); i>=0; i--))
do
	echo -n "${PID[i]} "
	kill -9 ${PID[i]}
done

echo ""
echo "Procesi koncani."
