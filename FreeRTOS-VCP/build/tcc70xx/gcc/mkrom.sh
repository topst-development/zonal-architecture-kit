#!/bin/bash

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


for ARGUMENT in "$@"
do
    KEY=$(echo $ARGUMENT | cut -f1 -d=)
    VALUE=$(echo $ARGUMENT | cut -f2 -d=)

    case "$KEY" in
            BOARD_NAME)    BOARD_NAME=${VALUE} ;;
            OUTPUT_PATH)   OUTPUT_PATH=${VALUE} ;;
            ENV_HOST)      ENV_HOST=${VALUE} ;;
            SECURE_UPDATE) SECURE_UPDATE=${VALUE} ;;
            *)
    esac
done

#make signed images
cd ../make_utility/vcpSignTool
make -j12
cd build-vcpSignTool
./vcpSignTool vcpcert ../CertificateChain/RTCU0001V_0124_MCU_CA.crt ../CertificateChain/RTCU0001V_0124_MCU_CS_CA.crt ../CertificateChain/RTCU0001V_0124_MCU_Firmware.crt ../CertificateChain/RTCU0001V_0124_MCU_HSMUpdate.crt ../../prebuilt/vcpcert.bin
./vcpSignTool vcphsm ../../prebuilt/hsm.hmac.bin ../../prebuilt/hsm.hmac.bin.signed ../CertificateChain/RTCU0001V_0124_MCU_CA.crt ../CertificateChain/RTCU0001V_0124_MCU_HSMBoot.crt ../../../../../tools/fwdn_vcp/vcp_fwdn.rom
./vcpSignTool vcpmcu ../../../gcc/output/r5_fw.rom ../../../gcc/output/r5_fw.rom.signed
cd ../../../gcc

SNOR_SIZE=4
UTILITY_DIR=../make_utility/tcc70xx_pflash_mkimage
OUTPUT_DIR=../../gcc/output
OUTPUT_FILE=tcc70xx_pflash_boot.rom

#source ./config.mk

pushd $UTILITY_DIR

#temporary use previous VCP tool
chmod 755 ./tcc70xx-pflash-mkimage

if [ "$ENV_HOST" -eq 1 ]
then
TOOL_NAME=tcc70xx-pflash-mkimage.exe
else
TOOL_NAME=tcc70xx-pflash-mkimage
fi

if [ "$SECURE_UPDATE" -eq 1 ]
then
./$TOOL_NAME -i ./tcc70xx.ecdsa.cfg -o $OUTPUT_DIR/$OUTPUT_FILE
else
./$TOOL_NAME -i ./tcc70xx.hmac.cfg -o $OUTPUT_DIR/$OUTPUT_FILE
fi


popd
