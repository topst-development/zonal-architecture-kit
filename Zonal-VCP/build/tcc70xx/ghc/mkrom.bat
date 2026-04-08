@echo off
echo %1
set OS_TYPE=%1

set IMAGE_NAME=R5-FW
set IMAGE_VERSION=0.0.0
set TARGET_ADDRESS=0x00000000
set UTILITY_DIR=..\make_utility
set PFLASH_MK_UTILITY_DIR=..\make_utility\tcc70xx_pflash_mkimage
set OUTPUT_DIR=output
set BUILD_OUTPUT_ROM=ghc-%OS_TYPE%\boot.mem
set OUTPUT_ROM=output\r5_fw.rom

set OUTPUT_FILE=tcc70xx_pflash_boot.rom

if not exist ".\%OUTPUT_DIR%" (
  mkdir .\%OUTPUT_DIR%
)

pushd %UTILITY_DIR%
call tcmktool.exe ..\ghc\%BUILD_OUTPUT_ROM% ..\ghc\%OUTPUT_ROM% %IMAGE_NAME% %IMAGE_VERSION% %TARGET_ADDRESS%
popd

pushd %PFLASH_MK_UTILITY_DIR%
call tcc70xx-pflash-mkimage.exe -i .\tcc70xx.ghc.cfg -o .\..\..\ghc\output\tcc70xx_pflash_boot.rom
popd