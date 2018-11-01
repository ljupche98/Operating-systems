#!/bin/bash

h=0
c=1
m=1
p=1
u=1

while true
do
	read -rsn1 -t 1 cmd

	if [ "$cmd" == "q" ]
	then
		exit 0
	elif [ "$cmd" == "h" ]
	then
		h=$(($h ^ 1))
	elif [ "$cmd" == "c" ]
	then
		c=$(($c ^ 1))
	elif [ "$cmd" == "m" ]
	then
		m=$(($m ^ 1))
	elif [ "$cmd" == "p" ]
	then
		p=$(($p ^ 1))
	elif [ "$cmd" == "u" ]
	then
		u=$(($u ^ 1))
	fi

	if [ "$h" == "1" ]
	then
		if [ "$cmd" == "h" ]
		then
			echo "Press q to quit"
			echo "Press h to toggle this help message"
			echo "Press c to toggle command display"
			echo "Press m to toggle memory display"
			echo "Press p to toggle CPU utilization display"
			echo "Press u to toggle user display"
			printf "\n\n"
		fi
	else
		allp=$(ps -aux | sort -nrk 3,3 | head -n 11 | grep "^[a-zA-Z+/ ]*[0-9][0-9]*" | tr -s " " | cut -d " " -f 2)

		printf "%-6s" "PID"

		if [ "$c" == "1" ]
		then
			printf "%-17s" "COMMAND"
		fi

		if [ "$m" == "1" ]
		then
			printf "%-6s" "%MEM"
		fi

		if [ "$p" == "1" ]
		then
			printf "%-6s" "%CPU"
		fi

		if [ "$u" == "1" ]
		then
			printf "%s" "USER"
		fi

		printf "\n"

		for pid in $allp
		do
			printf "%-6s" "$pid"

			if [ "$c" == "1" ]
			then
				printf "%-17s" "$(ps -p $pid -o comm | cut -d $'\n' -f 2)"
			fi

			if [ "$m" == "1" ]
			then
				printf "%-6s" "$(ps -p $pid -o %mem | cut -d $'\n' -f 2)"
			fi

			if [ "$p" == "1" ]
			then
				printf "%-6s" "$(ps -p $pid -o %cpu | cut -d $'\n' -f 2)"
			fi

			if [ "$u" == "1" ]
			then
				printf "%s" "$(ps -p $pid -o user | cut -d $'\n' -f 2)"
			fi

			printf "\n"
		done

		printf "\n-----------------------------------------\n\n"
	fi
done
