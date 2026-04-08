#!/bin/bash

# EULA
if [ -f .eula_accepted ]; then
	ACCEPT_EULA=1
else
	ACCEPT_EULA=0
fi

OPTIND=1

while getopts "e" opt; do
	case "$opt" in
		e)
			ACCEPT_EULA=1
			;;
	esac
done

shift $((OPTIND-1))

if [ $ACCEPT_EULA -eq 0 ];then
	TERMINAL_SIZE=($(stty -a | tr ';' '\n' | grep -E "rows|columns" | cut -d ' ' -f3))
	TERMINAL_SIZE[0]=$((TERMINAL_SIZE[0] -= 4))
	TERMINAL_SIZE[1]=$((TERMINAL_SIZE[1] -= 4))
	TERMINAL_SIZE=${TERMINAL_SIZE[@]}

	if (whiptail \
		--title "End User License Agreement" \
		--textbox "./tools/EULA.txt" \
		--scrolltext \
		$TERMINAL_SIZE \
		--ok-button "Proceed to confirm" \
		); then
		if (whiptail \
			--title "End User License Agreement" \
			--yesno "Please confirm to use the TOPST SDK" 10 50 \
			--yes-button Accept \
			--no-button Reject\
			); then
			ACCEPT_EULA=1
		else
			>&2 echo [-] EULA rejected
			exit
		fi
	else
		>&2 echo [-] EULA rejected
		exit
	fi

	unset TERMINAL_SIZE

	touch .eula_accepted
fi

cat <<EOL
cd build/tcc70xx/gcc/
make
EOL
