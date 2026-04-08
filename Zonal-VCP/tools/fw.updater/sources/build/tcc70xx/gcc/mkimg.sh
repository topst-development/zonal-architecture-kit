# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : mk_tc_img.sh
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

#/bin/sh
for ARGUMENT in "$@"
do
    KEY=$(echo $ARGUMENT | cut -f1 -d=)
    VALUE=$(echo $ARGUMENT | cut -f2 -d=)

    case "$KEY" in
        TOOL_PATH)      TOOL_PATH=${VALUE} ;;
        INPUT_PATH)     INPUT_PATH=${VALUE} ;;
        OUTPUT_PATH)    OUTPUT_PATH=${VALUE} ;;
        IMAGE_VERSION)  IMAGE_VERSION=${VALUE} ;;
        TARGET_ADDRESS) TARGET_ADDRESS=${VALUE} ;;
        *)
    esac
done

MKTOOL_INPUT=$INPUT_PATH/boot.bin
MKTOOL_OUTPUT=$OUTPUT_PATH/updater.rom
MKTOOL_NAME=R5-SUB-FW
MKTOOL_SOC_NAME=70xx

chmod 755 $TOOL_PATH/tcmktool
$TOOL_PATH/tcmktool $MKTOOL_INPUT $MKTOOL_OUTPUT $MKTOOL_NAME $IMAGE_VERSION $TARGET_ADDRESS $MKTOOL_SOC_NAME

