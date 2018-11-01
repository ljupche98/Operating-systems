#!/bin/bash

function gcd {
	if (($1 == 0))
	then
		echo $2
	else
		echo $(gcd $(($2 % $1)) $(($1)))
	fi
}

function leap {
	if (($1 % 400 == 0))
	then
		echo "je"
	elif (($1 % 100 == 0))
	then
		echo "ni"
	elif (($1 % 4 == 0))
	then
		echo "je"
	else
		echo "ni"
	fi
}

function fib {
	if (($1 == 0))
	then
		echo 0
	else
		f=1
		s=1
		for ((i=2; i<$1; i++))
		do
			t=$(($f))
			f=$(($f + $s))
			s=$(($t))
		done
		echo $f
	fi
}

function UI {
	if [ $(cat /etc/passwd | grep ^$1:) ]
	then
		r=0

		if [ $(cat /etc/passwd | grep ^$1: | cut -d : -f 3) == $(cat /etc/passwd | grep ^$1: | cut -d : -f 4) ]
		then
			r=$(($r + 2))
			# echo -n "enaka"
		fi

		if [ -d "/home/$1" ] || [ -d "/home/uni/$1" ]
		then
			r=$(($r + 1))
			# echo -n "obstaja"
		fi

		if (($r == 3))
		then
			echo -n "enaka obstaja "
		elif (($r == 2))
		then
			echo -n "enaka "
		elif (($r == 1))
		then
			echo -n "obstaja "
		fi

		echo -n $(($(groups $1 | grep -o " " | wc -l) - 1))

	else
		echo -n "err"
	fi

	echo
}

function print_type {
	  if [ -f "$1" ]
	then
		echo -n "FILE  "
	elif [ -L "$1" ]
	then
		echo -n "LINK  "
	elif [ -c "$1" ]
	then
		echo -n "CHAR  "
	elif [ -b "$1" ]
	then
		echo -n "BLOCK "
	elif [ -p "$1" ]
	then
		echo -n "PIPE  "
	elif [ -S "$1" ]
	then
		echo -n "SOCK  "
	elif [ -d "$1" ]
	then
		echo -n "DIR   "
	fi
}

function search_rec {

	for f in $(ls "$1") # $1/*
	do
		# if [ "$(echo $f)" == "$(echo $f | grep ^/[a-zA-Z0-9/.\ ]*$)" ]
		# if [ -e "$1/$f" ]
		if [ "${1##/*/}" != "$f" ]
		then
			for ((i=0; i<=$(($2)); i++))
			do
				echo -n "----"
			done

			print_type "$1/$f"

			echo "${f##/*/}"

			if [ -d "$1/$f" ] # $1/$f
			then
				if (($(($2)) < $(($3)) - 1))
				then
					if [ f == f ]
					then
						search_rec "$1/$f" "$(($(($2)) + 1))" "$3"
					fi
				fi
			fi
		fi
	done
}

function space_rec {

	fn=0
	sn=0

	for f in $(ls "$1")
	do
		if [ -d "$1/$f" ]
		then
			if (($(($2)) < $(($3)) - 1))
			then
				rr=$(space_rec "$1/$f" $(($(($2)) + 1)) $3)
				fn=$(($((${rr%%<>[0-9]*})) + $fn))
				sn=$(($((${rr##[0-9]*<>})) + $sn))
			fi
		fi

		fn=$(($(stat "$1/$f" | sed -n '2p' | tr -s " " | cut -d " " -f 3) + $fn))
		sn=$(($(stat "$1/$f" | sed -n '2p' | tr -s " " | cut -d " " -f 5) + $sn))
	done

	echo "$fn<>$sn"
}

function er {
	echo "Napacna uporaba skripte!"
	echo "Uporaba: ./Naloga.sh akcija parametri"
	exit 42
}

  if (($# == 0))
then
	er
elif [ $1 != "pomoc" ] && [ $1 != "status" ] && [ $1 != "leto" ] && [ $1 != "fib" ] && [ $1 != "userinfo" ] && [ $1 != "tocke" ] && [ $1 != "drevo" ] && [ $1 != "prostor" ]
then
	er
elif [ $1 == "pomoc" ]
then
	echo "Uporaba: $0 akcija parametri"
elif [ $1 == "status" ]
then
	exit $(gcd $2 $3)
elif [ $1 == "leto" ]
then
	shift
	for ly in $@
	do
		echo "Leto $ly $(leap $ly) prestopno."
	done
elif [ $1 == "fib" ]
then
	shift
	for fn in $@
	do
		echo "$fn: $(fib $fn)"
	done
elif [ $1 == "userinfo" ]
then
	shift
	for un in $@
	do
		echo "$un: $(UI $un)"
	done
elif [ $1 == "tocke" ]
then
	SC=$((0))
	SS=$((0))
	RANDOM=42

	while read line
	do
		if [ "$(echo $line | grep -c ^#)" -eq 0 ]
		then
			if (($(($(echo $line | grep " " | grep -o " " | wc -l))) >= 3))
			then
				done=0
				SID=$(echo $line | cut -d " " -f 1)
				SUM=$(($(($(echo $line | cut -d " " -f 2))) + $(($(echo $line | cut -d " " -f 3))) + $(($(echo $line | cut -d " " -f 4)))))

				if (($(($(echo $line | grep " " | grep -o " " | wc -l))) == 4))
				then
					if [ $(echo $line | cut -d " " -f 5) == p ] || [ $(echo $line | cut -d " " -f 5) == P ]
					then
						done=1
						SUM=$(($SUM / 2))
					fi
				fi

				if [ $done == 0 ]
				then
					if [ $(echo $SID | grep ^[0-9][0-9]14[0-9]*$) ]
					then
						SUM=$(($SUM + $((($RANDOM % 5) + 1))))
					fi
				fi

				if (($SUM >= 50))
				then
					SUM=$((50))
				fi

				echo "$SID: $SUM"
				SC=$(($SC + 1))
				SS=$(($SS + $SUM))
			fi
		fi
	done

	echo "St. studentov: $SC"
	echo "Povprecne tocke: $(($SS / $SC))"
elif [ $1 == "drevo" ]
then
	WD=$PWD
	LVL=3

	if (($# >= 3))
	then
		if [ -e "$2" ]
		then
			WD=$2
		else
			er
		fi

		if [ "$(echo $3)" == "$(echo $3 | grep ^[0-9]*$)" ]
		then
			LVL=$3
		else
			er
		fi
	fi

	if (($# == 2))
	then
		if [ "$(echo $2)" == "$(echo $2 | grep ^[0-9]*$)" ]
		then
			LVL=$2
		else
			WD=$2
		fi
	fi

	IFS=$'\n'
	print_type $WD
	echo $WD
	search_rec "$WD" "0" "$LVL"
	unset IFS
elif [ $1 == "prostor" ]
then
	WD=$PWD
	LVL=3

	if (($# >= 3))
	then
		WD=$2
		LVL=$3
	fi

	if (($# == 2))
	then
		if [ "$(echo $2)" == "$(echo $2 | grep ^[0-9]*$)" ]
		then
			LVL=$2
		else
			WD=$2
		fi
	fi

	fnn=$(stat $WD | sed -n '2p' | tr -s " " | cut -d " " -f 3)
	snn=$(stat $WD | sed -n '2p' | tr -s " " | cut -d " " -f 5)

	IFS=$'\n'
	rr=$(space_rec "$WD" "0" "$LVL")
	echo "Velikost: $((${rr%%<>[0-9]*} + $fnn))"
	echo "Blokov: $((${rr##[0-9]*<>} + $snn))"
	echo "Prostor: $(($((${rr##[0-9]*<>} + $snn)) * 512))"
	unset IFS
fi
