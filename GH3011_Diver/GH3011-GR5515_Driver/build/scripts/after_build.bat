::=============================================================================
:: Developer should configure BIN_PATH, TOOL_PATH, TARGET and RUN_ADDR
::=============================================================================
::set the path to your Keil installation folder
@echo off

@if "%1"=="" (
echo Please input the app name!
GOTO:EOF
)

@if "%2"=="" (
@set BIN_PATH=C:\Keil_v5\ARM\ARMCC\Bin\
echo Use keil install path by default -- C:\Keil_v5\ARM\ARMCC\Bin\
) else (
@set BIN_PATH=%2\ARM\ARMCC\Bin
)

@set SWD_DISENABLE=0
@set ENCRY_ENABLE=1
::default configuration of the efuse
::fastest -----slowest
@set EB_S64M_F64M_M0_P1=0X03D60281  
@set 6B_S48M_F32M_M3_P1=0X02D65A83
@set BB_S32M_F32M_M3_P1=0X03765A8B
@set 0B_S24M_F16M_M0_P0=0X0016A287
@set 03_S16M_F16M_M3_P0=0X00069A89
::default 	
@set CONFIG=0X0016A286 
::set the path to SDK tools folder
@set TOOL_PATH=..\..\..\..\..\build\binaries
@set EFUSE_CONFIG_BIN=Efuse_SWD_Disable_%SWD_DISENABLE%_Encry_Mode_%ENCRY_ENABLE%_%CONFIG%
:: TARGET must be same with project options->Output->Name of Executable
@set TARGET=%1

@set OBJ_DIR_PATH=.\Objects
@set LISTING_DIR_PATH=.\Listings

@if "%3"=="" (
@set OUTPUT_DIR_PATH=.\build
) else (
@set OUTPUT_DIR_PATH=%2
)

@set FW_ENCRYPT_SIGN_TOOL_PATH=..\..\..\..\..\build\binaries\encrypt_tools
@set PRODUCT_ID=11111
@set PRODECT_NAME=goodix_ble_test
@set LOAD_ADDR=0x01002000
@set FW_KEY_SAVE_PATH=%FW_ENCRYPT_SIGN_TOOL_PATH%\ProductInfo\%PRODUCT_ID%\%PRODECT_NAME%
for /R %FW_KEY_SAVE_PATH% %%i in (*.fwkey) do @set FW_KEY=%%~ni


@set TARGET_APP=%TARGET%_fw
@set TARGET_APP_ENCRY=%TARGET%_encrypt

@if not exist  %OUTPUT_DIR_PATH% (
md %OUTPUT_DIR_PATH%
) else (
del /q/s %OUTPUT_DIR_PATH%\* >nul
)

::Generate the bin file with fromelf.exe
%BIN_PATH%\fromelf.exe --bin --output %OBJ_DIR_PATH%\%TARGET%.bin %OBJ_DIR_PATH%\%TARGET%.axf

::Generate the assembly code with fromelf.exe
%BIN_PATH%\fromelf.exe --text -c --output %OBJ_DIR_PATH%\%TARGET%.s %OBJ_DIR_PATH%\%TARGET%.axf

copy %OBJ_DIR_PATH%\%TARGET%.bin %OUTPUT_DIR_PATH%\%TARGET%.bin >nul

::Generate system all output files
%TOOL_PATH%\ble_tools.exe --mode=gen_fw  --cfg=..\Src\config\custom_config.h  --bin=%OUTPUT_DIR_PATH%\%TARGET%.bin --out_dir=%OUTPUT_DIR_PATH% --app_name=%TARGET_APP%  --dap

%FW_ENCRYPT_SIGN_TOOL_PATH%\fw_encrypt_signature.exe -e %FW_KEY_SAVE_PATH%\fwkey.bin -s %FW_KEY_SAVE_PATH%\privateKey.bin -p %FW_KEY_SAVE_PATH%\publicKey.bin -r %FW_KEY_SAVE_PATH%\random.bin -f %OUTPUT_DIR_PATH%\%TARGET_APP%.bin -i %PRODUCT_ID% -o %OUTPUT_DIR_PATH%\%TARGET_APP_ENCRY%.bin >nul

%TOOL_PATH%\ble_tools.exe --mode=encrypt --cfg=%CONFIG% --keyfile=%FW_KEY_SAVE_PATH%\smt.bin --swd=%SWD_DISENABLE% --enc=%ENCRY_ENABLE% --output=%OUTPUT_DIR_PATH%\%EFUSE_CONFIG_BIN%.bin >nul
%TOOL_PATH%\ble_tools.exe --mode=bin2hex  --bin=%OUTPUT_DIR_PATH%\%TARGET_APP_ENCRY%.bin  --hex=%OUTPUT_DIR_PATH%\%TARGET_APP_ENCRY%.hex --load_addr=0x01002000 >nul

copy %OUTPUT_DIR_PATH%\%TARGET_APP%.bin  %OUTPUT_DIR_PATH%\load_fw.bin >nul
copy %OUTPUT_DIR_PATH%\%TARGET_APP_ENCRY%.bin  %OUTPUT_DIR_PATH%\load_fw_encrypt.bin >nul

%TOOL_PATH%\ble_tools.exe --mode=merge_bin --cfg=..\..\..\..\..\build\scripts\simulation_unenc.txt --in_dir=%OUTPUT_DIR_PATH% --bin=%OUTPUT_DIR_PATH%\%TARGET_APP%_sim.bin  --bin_size=0x80000 >nul

del /q/s %OUTPUT_DIR_PATH%\*.tmp >nul
del /q/s %OUTPUT_DIR_PATH%\info.* >nul
del /q/s %OUTPUT_DIR_PATH%\header.bin >nul

del .\x >nul 2>nul
del .\y >nul 2>nul
del .\k1 >nul 2>nul
del .\k2 >nul 2>nul

::copy the bin,axf,sct... to output dir
copy %OBJ_DIR_PATH%\%TARGET%.axf %OUTPUT_DIR_PATH%\%TARGET%.axf >nul
copy %LISTING_DIR_PATH%\%TARGET%.map %OUTPUT_DIR_PATH%\%TARGET%.map >nul
copy %OBJ_DIR_PATH%\%TARGET%.s %OUTPUT_DIR_PATH%\%TARGET%.s >nul
copy %OUTPUT_DIR_PATH%\%TARGET_APP%.hex %OUTPUT_DIR_PATH%\load_app.hex >nul
copy %OUTPUT_DIR_PATH%\%TARGET_APP_ENCRY%.hex %OUTPUT_DIR_PATH%\load_app_encrypt.hex >nul

echo Firmware has been successfully generated!

