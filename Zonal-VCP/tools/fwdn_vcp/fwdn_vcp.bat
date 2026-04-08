@ECHO OFF

set ROM_PATH=.\

fwdn.exe --write %ROM_PATH%tcc70xx_pflash_boot_2M_ECC.rom --fwdn .\vcp_fwdn.rom
