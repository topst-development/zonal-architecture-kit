# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : mkdrom.linux.es.sh
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
            BOARD_NAME)    BOARD_NAME=${VALUE} ;;
            OUTPUT_PATH)   OUTPUT_PATH=${VALUE} ;;
            *)
    esac
done



SNOR_SIZE=4
UTILITY_DIR=../make_utility/tcc70xx_pflash_mkimage
OUTPUT_DIR=../../gcc/output
OUTPUT_FILE=tcc70xx_pflash_boot.rom

#source ./config.mk

pushd $UTILITY_DIR

#temporary use previous VCP tool
chmod 755 ./tcc70xx-pflash-mkimage


./tcc70xx-pflash-mkimage -i ./tcc70xx.cfg -o $OUTPUT_DIR/$OUTPUT_FILE
			
		

popd

