#/bin/sh
for ARGUMENT in "$@"
do
	KEY=$(echo $ARGUMENT | cut -f1 -d=)
	VALUE=$(echo $ARGUMENT | cut -f2 -d=)

	case "$KEY" in
		TOOL_PATH)			TOOL_PATH=${VALUE} ;;
		OUTPUT_PATH)    OUTPUT_PATH=${VALUE} ;;
		*)
	esac
done

chmod 755 $TOOL_PATH/mkmemsize/*
python3 $TOOL_PATH/mkmemsize/main.py
