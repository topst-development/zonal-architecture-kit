@ECHO OFF
cls
REM NAME FWDN_VCP.BAT
REM FUNCTION: This is a batch file to help with firmware download.
REM It supports 3 functions. (WRITE, DUMP, LOW FORMAT)

set CURPATH=%cd%
set FWDN_PATH=%CURPATH%\..\..\..\..\tools\fwdn_v8
set OUTPUT_PATH=%CURPATH%\output
	
echo.
echo ======================
echo l	  RUN        l
echo ======================
echo   1. Write Firmware
echo   2. Dump Firmware
echo   3. LowFormat
echo   4. Exit
echo.
set /p choice=Selection:
echo.

:switch all
goto :case-%choice%

:case-1
::::::::::::::::::::::::::::::
::      WRITE Firmware      ::
::::::::::::::::::::::::::::::
call :SeleBank
call :SelePort
%FWDN_PATH%\fwdn.exe --write %OUTPUT_PATH%\tcc70xx_pflash_boot.rom --fwdn %FWDN_PATH%\vcp_fwdn.rom --bank %bank% %1
goto :case-4

:case-2
:::::::::::::::::::::::::::::
::      DUMP Firmware      ::
:::::::::::::::::::::::::::::
call :SeleBank
call :EnterFileName
call :SelePort
%FWDN_PATH%\fwdn.exe --read %DUMP_FILE% --fwdn %FWDN_PATH%\vcp_fwdn.rom --hsm %OUTPUT_PATH%\tcc70xx_pflash_boot.rom --bank %bank% %1
goto :case-4

:case-3
::::::::::::::::::::::::::::::::
::      LOW_FORMAT FLASH      ::
::::::::::::::::::::::::::::::::
call :SeleBank
call :SelePort
%FWDN_PATH%\fwdn.exe --low-format --hsm %OUTPUT_PATH%\tcc70xx_pflash_boot.rom --fwdn %FWDN_PATH%\vcp_fwdn.rom --bank %bank% %1
goto :case-4

:case-4
::::::::::::::::::::
::      EXIT      ::
::::::::::::::::::::
echo GoodBye
exit /b

:SeleBank 
echo ---------------------
echo    Selection bank 
echo ---------------------
echo   1. bank_0
echo   2. bank_1
echo.
set /p var=Selection:
if %var%==1 set bank=0
if %var%==2 set bank=1
echo.
exit /b

:SelePort
echo --------------------
echo    Selection Port 
echo --------------------
for /f "tokens=1" %%a in ('mode') do (
	echo     %%a | find "COM"
)
echo.
exit /b

:EnterFileName
echo ---------------------
echo    Enter File Name  
echo ---------------------
set /p DUMP_FILE=   :
echo.
exit /b
