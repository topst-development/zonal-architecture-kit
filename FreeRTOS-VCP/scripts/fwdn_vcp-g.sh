#!/bin/bash

set -e

PROJECT_DIR="$HOME/zonal-architecture-kit/FreeRTOS-VCP"

FWDN_TOOL="$PROJECT_DIR/tools/fwdn_vcp/fwdn"
FWDN_ROM="$PROJECT_DIR/tools/fwdn_vcp/vcp_fwdn.rom"
TARGET_ROM="$PROJECT_DIR/build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom"

sudo "$FWDN_TOOL" \
    --fwdn "$FWDN_ROM" \
    -w "$TARGET_ROM"
