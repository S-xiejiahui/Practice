:: 关闭 echo 打印，设置变量延时，便于获取变量值时不出错，并设置运行窗口名
@echo off&setlocal enabledelayedexpansion&title Pclint-checking

rem ==========================================================================================
rem                                      Pclint 检测脚本
rem ==========================================================================================

:: 记录时间戳，计算运行时长
set t=%time%
:: 当前时间
echo Local Time: %date:~0,4%-%date:~5,2%-%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%

rem ==========================================================================================
rem                                   平台包以及检测路径的设置
rem ==========================================================================================

:: 获取 UGW6.0_HomeCoverage 目录路径
cd %cd%\..\..
set UGW6_0=%cd%&cd /d %~dp0
:: 要检查的代码的绝对路径
set target_directory=%UGW6_0%\prod
:: platform 文件夹路径
set platform_path=%UGW6_0%\platform
:: 要解压的平台包的名字，不带后缀
set release_package=rtl8197h_8832br_8367rb_11ax_release
set wifi_release_package=rtl8197h_8832br_8367rb_11ax_wifi_release
:: prod 公共库函数文件夹路径(所有头文件路径)
set prod_common_path=%UGW6_0%\prod\common
:: prod目录下的要检测的文件夹名称，以空格隔开，或者使用^加回车换行
set c_file_path=internet_detec single_led ate common agent\netctrl agent\multiwan httpd\goahead\src
:: 设置生成报告的类型(暂时只支持 txt xml)
set file_type=txt
:: targets 所在目录
set product_targets=%UGW6_0%\targets
:: targets 下产品的目录，即被检测的产品的名字
set product_name=ax2pro_nor rx27pro
rem ==========================================================================================
rem          解压平台包(以下处理是针对 bz2 格式的压缩包，其他格式可能不需要解压缩两遍)
rem ==========================================================================================

:: pclint 解压平台包到该文件夹,和一些未加入检测的零散头文件
set pclint_include=%cd%\Pclint_Include
:: 7zip 解压工具的路径
set un7zip_path=%pclint_include%\7-Zip\7z
::	bcm6756_11ax_release 平台包的路径
set platfrom_release_package_path=%platform_path%\%release_package%.bz2
:: bcm6756_11ax_wifi_release 平台包的路径
set platfrom_wifi_release_package_path=%platform_path%\%wifi_release_package%.bz2
:: 临时存放解压一次后的文件的路径
set temp_path=%pclint_include%\tmp
set tab=	
:: 调用解压平台包函数
call :Decompress_platform_package
:: 显示 pclint 界面版本信息
call :Windows_interface

rem ==========================================================================================
rem                                     确保文件夹存在
rem ==========================================================================================

:: pclint 检测程序以及 lnt 配置文件所在路径
set pclint_install_dir=%cd%\lint
:: 生成报告所存放的文件夹 (此处需要绝对路径，或者相对于 lint\lint-nt.exe 的路径，而非相对于 pclint.bat )
:: 可以指定生成路径，将下面 %cd%\Report 改为你主机的目录的绝对路径
set pclint_report_dir=%cd%\Pclint_Report
:: 存放所有待检测的 .c 文件的路径信息的 lnt 文件集
set pclint_filelist_dir=%cd%\Pclint_FileList
:: 删除上一次寻找的头文件路径，保证每次都是最新的路径
del %pclint_filelist_dir%\*.lnt /q >nul 2>nul
:: pclint 头文件和规则 lnt 存放路径
set pclint_rules_dir=%cd%\Pclint_Rules
:: 调用确保文件存在函数
call :Make_sure_files_exist

rem ==========================================================================================
rem          				          获取产品开启的宏
rem ==========================================================================================

:: 获取编译产品 targets 目录下 config.mk 文件所有开起的宏(product_name 指定的产品)
call :get_product_model_macro_definition

rem ==========================================================================================
rem     获取自身的头文件路径和.c文件的路径、获取平台头文件路径、获取 prod 公共头文件路径
rem ==========================================================================================

echo.
echo ==============================================================================
echo                 ^(2^)Gets the path of each header file, Loading...
echo ==============================================================================
echo.
echo Automatic searching for header file path, please wait a minutes ...
echo.

:: 用于保存文件夹名和文件个数
set "report_filename="

:: 获取待检测文件夹下的 .c .h 文件路径
call :Get_files_from_their_folders
:: 忽略其中一些 .c 文件的检测
call :Undetected_list
:: 获取 platform 公共头文件路径
call :Gets_platform_header_file_path
:: 获取 prod/common 公共头文件路径
call :Gets_common_header_file_path
:: 将 include_header.lnt 文件中所写的头文件拷贝到 pclint_include\include 文件夹下，作为库文件
call :xcopy_header_to_include_file

echo.
echo %tab%-----[ Include Search completed ]-----
echo.

rem ==========================================================================================
rem          设置以下文件夹为库文件，且不报错、设置检测规则的路径、和文件输出格式的选择
rem ==========================================================================================

:: 设置这三个文件为库文件、库文件不报错(此处的%cd%是当前目录)
del %pclint_rules_dir%\public_lib.lnt /q >nul 2>nul
call :set_library_path

:: 判断文件输出格式，根据 file_type 设置获取生成报告的格式
if %file_type% equ xml (
    set system_option=-i%pclint_install_dir% std incl env-xml
) else (
    set file_type=txt
    set system_option=-i%pclint_install_dir% std incl env-jenkins
)

:: 设置 pclint 检测规则

set public_option=%pclint_rules_dir%\public_rules.lnt %pclint_rules_dir%\public_lib.lnt

set prod_platform_inc=%pclint_filelist_dir%\inc_platform.lnt %pclint_filelist_dir%\inc_prod_common.lnt

rem ==========================================================================================
rem                                          开始检测
rem ==========================================================================================

for %%p in (%product_name%) do (
	call :Windows_interface
	echo ==============================================================================
	echo              ^(3^)^(%%p^)Start checking for all ^(.c files^), wait...
	echo ==============================================================================
	echo.
	if not exist "%pclint_report_dir%\%%p" (
		mkdir %pclint_report_dir%\%%p
	) else (
		del %pclint_report_dir%\%%p_all.%file_type% /q >nul 2>nul
	)
	del %pclint_report_dir%\%%p\*.%file_type% /q >nul 2>nul
	set product_name_tmp=%%p
	call :start_checking_product_all_c_files
)

rem ==========================================================================================
rem                                     打印检测结果
rem ==========================================================================================

:: 计算检测文件个数
set total_number=0
if exist %pclint_filelist_dir%\*_src.lnt (
	for %%f in (%pclint_filelist_dir%\*_src.lnt) do (
		for /f "tokens=*" %%i in ('findstr /n . %%f^|find /c /v ""') do set number=%%i
		set /a total_number+=!number!
	)
)

:: 计算运行总时长
call :time_used "%t%" "%time%" used_time
echo ============================= Finish =================================
echo ===                   !total_number! files were tested                        ===
echo ===            Open the folder and view the check report           ===
echo ===                 [filename: Pclint_Report ]                     ===
echo ===            The total available: %used_time%           === 
echo ======================================================================
echo.

:: 判断 pclint 是在服务器运行还是本地，服务器则运行完就关闭
if %file_type% neq xml (
    exit 0
)

if %file_type% equ xml (
	for %%p in (%product_name%) do (
		:: 检测 error 和 warning 的个数
		call :get_error_warning %pclint_report_dir%\%%p_all.%file_type% ErrorNumer WarningNumer
		echo.
		echo ===========================    %%p    ===================================
		echo.
		echo %tab%%%p all: %tab%^[ ErrorNumer:%tab%!ErrorNumer!   WarningNumer:%tab%!WarningNumer! ^]
		echo.
		echo ===============================================================================
		if "!ErrorNumer!" == "0" ( if "!WarningNumer!" == "0" del %pclint_report_dir%\%%p_all.%file_type%)
		for %%f in (Pclint_Report\%%p\*.xml) do (
			set xml_path=%%f
			for %%y in ( "!xml_path:\= !" ) do set "path_tmp=%%~y"
			for %%y in (!path_tmp!) do set "filename=%%y"

			if "!filename!" neq "all.%file_type%" (
				call :get_error_warning %pclint_report_dir%\%%p\!filename! ErrorNumer WarningNumer
				echo %tab%!filename!   %tab%%tab%%tab%{  Error: !ErrorNumer!, %tab%Warning: !WarningNumer!  }
				echo ----------------------
				if "!ErrorNumer!" == "0" ( if "!WarningNumer!" == "0" del %%f)
			)
		)
		echo ------------------------------------------------------------------------------
		echo.
	)
    echo.
)

pause&exit 0

:start_checking_product_all_c_files
	if %file_type% equ xml (
		:: 将所有 error 和 warning 汇总到 all.xml 中
		echo ^<^?xml version="1.0" encoding="UTF-8"^?^> >> %pclint_report_dir%\!product_name_tmp!_all.%file_type%
		echo ^<doc^> >> %pclint_report_dir%\!product_name_tmp!_all.%file_type%
	)

	set objIndex=0
	for  %%j in ( !report_filename! ) do (
		set /a objIndex+=1
		echo [!objIndex!] checking %%j
	:: 检测目录下的所有头文件路径
		set check_head=%pclint_filelist_dir%\%%j_inc.lnt
	:: 包含 .c 文件的 lnt 的路径
		set check_c_file=%pclint_filelist_dir%\%%j_src.lnt
	:: 该产品所开启的宏定义的宏的内容的路径
		set define_config_mk_file=%pclint_filelist_dir%\!product_name_tmp!_config_mk.lnt
	:: 各检测文件夹对应的 option 文件的路径
		set file_self_option_rule=%pclint_rules_dir%\option_%%j.lnt
		if not exist !file_self_option_rule! ( echo. > !file_self_option_rule! )
	:: 所有 option
		set all_option=%system_option% -u %public_option% !file_self_option_rule! !check_head! !define_config_mk_file! %prod_platform_inc% !check_c_file!

		if %file_type% equ xml (
	:: 检测
			%pclint_install_dir%\lint-nt.exe !all_option! >> %pclint_report_dir%\!product_name_tmp!\%%j.%file_type%
			echo.
	:: 每检测一个文件，就打印出 error warning 个数
			call :get_error_warning %pclint_report_dir%\!product_name_tmp!\%%j.%file_type% ErrorNumer WarningNumer
			echo ------------------------------------------------------------------------------
			echo %tab%%%j   %tab%{  Error: !ErrorNumer!, %tab%Warning: !WarningNumer!  }
			echo ------------------------------------------------------------------------------
	:: 将单个 xml 中的内容汇总到 all.xml 中
			findstr .* %pclint_report_dir%\!product_name_tmp!\%%j.%file_type%|findstr /vb "<doc>"|findstr /v "</doc>"|findstr /v "<?xml version="1.0" encoding="UTF-8"?>" >> %pclint_report_dir%\!product_name_tmp!_all.%file_type%
		) else (
			%pclint_install_dir%\lint-nt.exe !all_option!
			echo.
		)
		echo.
	)

	if %file_type% equ xml (
		echo ^</doc^> >> %pclint_report_dir%\!product_name_tmp!_all.%file_type%
	)
)
goto :eof

rem ==========================================================================================
rem                               函数 :Windows_interface
rem ==========================================================================================

:Windows_interface
echo.
echo.
echo 	 /******************************************************
echo 	 *                 Pclint Running                    * *
echo 	 ****************************************************  *
::@echo  *     author        : XieJiaHui                    *  *
echo 	 *     group         : ISP                          *  *
echo 	 *     Created       : 2021/12/16                   *  *
echo 	 *     Update        : 2022/5/20                    *  *
echo 	 ****************************************************  *
echo 	 *     File Name     : pclint.bat                   *  *
echo 	 *     Version       : Initial Draft (V2.0)         *  *
echo 	 *     environment   : Windows                      *  *
echo 	 ****************************************************/
echo.
echo.
echo Pclint-Testing in progress, please wait a minutes, May have to wait 2 - 4 min ...
echo.
goto :eof

rem ==========================================================================================
rem                             函数 :Decompress_platform_package
rem ==========================================================================================

:Decompress_platform_package
:: 解压 bcm6756_11ax_release 压缩包(bz2格式需要解压两遍)
if not exist %pclint_include%\%release_package% (
    echo.
    echo ======================================================================
    echo         Decompressing the platform release package, Loading...
    echo ======================================================================
    md %temp_path% >nul 2>nul
    %un7zip_path% x %platfrom_release_package_path% -o%temp_path%
    echo ------------------------------------------------
    echo unpacking, It'll take a while, Loading...
    %un7zip_path% x %temp_path%\%release_package% -o%temp_path%\.. >nul 2>nul
    for /f %%p in ('dir /b /ad "%pclint_include%\%release_package%"') do (
        if "%%p" neq "include" (
            echo delete %pclint_include%\%release_package%\%%p
            rd /s /q %pclint_include%\%release_package%\%%p
        ) else echo Add %pclint_include%\%release_package%\include
    )
    del %temp_path%\%release_package% /q >nul 2>nul
)

:: 解压 bcm6756_11ax_wifi_release 压缩包(bz2格式需要解压两遍)
if not exist %pclint_include%\%wifi_release_package% (
	echo.
	echo ======================================================================
	echo           Decompressing the wifi_release package, Loading...
	echo ======================================================================
	md %temp_path% >nul 2>nul
	%un7zip_path% x %platfrom_wifi_release_package_path% -o%temp_path%
	echo ------------------------------------------------
	echo unpacking, It'll take a while, Loading...
	%un7zip_path% x %temp_path%\%wifi_release_package% -o%temp_path%\.. >nul 2>nul
	for /f %%p in ('dir /b /ad "%pclint_include%\%wifi_release_package%"') do (
		if "%%p" neq "include" (
			echo delete %pclint_include%\%wifi_release_package%\%%p
			rd /s /q %pclint_include%\%wifi_release_package%\%%p
		) else echo Add %pclint_include%\%wifi_release_package%\include
	)
	del %temp_path%\%wifi_release_package% /q >nul 2>nul
)

rd %temp_path% /q >nul 2>nul
goto :eof

rem ==========================================================================================
rem          				     自动获取 tsfile.mk 文件对应产品开启的宏
rem ==========================================================================================
:get_product_model_macro_definition
echo.
echo ==============================================================================
echo                  ^(1^)Getting macro definition for this product...
echo ==============================================================================
echo.

for %%u in (%product_name%) do (
	echo ==========^> [ %%u : Get config.mk success ]
	set product_config_mk_path=%product_targets%\%%u\config.mk
	if exist !product_config_mk_path! (
		for /f "delims=&" %%j in ( !product_config_mk_path! ) do (
			echo -D%%j | findstr /v "#" >> %pclint_filelist_dir%\temp.lnt
		)
	)
	for /f "tokens=1 delims== " %%i in ( %pclint_filelist_dir%\temp.lnt ) do (
		echo %%i >> %pclint_filelist_dir%\%%u_config_mk.lnt
	)
	del %pclint_filelist_dir%\temp.lnt /q >nul 2>nul
)


goto :eof

rem ==========================================================================================
rem                             函数 :Make_sure_files_exist
rem ==========================================================================================

:Make_sure_files_exist
:: 确保这四个文件夹存在，以防误删文件导致运行报错
if not exist "%pclint_filelist_dir%" (
    mkdir %pclint_filelist_dir%
)

if not exist "%pclint_report_dir%" (
    mkdir %pclint_report_dir%
)

if not exist "%pclint_rules_dir%" (
    mkdir %pclint_rules_dir%
)
goto :eof

rem ==========================================================================================
rem                             函数 :Get_files_from_their_folders
rem ==========================================================================================

:Get_files_from_their_folders
for %%u in (%c_file_path%) do (
	:: 文件的绝对路径
	set absolute_path=%target_directory%\%%u
	if exist !absolute_path! (
		echo !absolute_path!
		:: 字符串截取，获取文件夹名字(如prod\agent,得到agent)
		for %%y in ( "!absolute_path:\= !" ) do set "path_tmp=%%~y"
		for %%y in ( !path_tmp! ) do set "filename=%%y"
		
		:: 把 cgi 名字改成 httpd
		if "!filename!" == "src" ( if "%%u" == "httpd\goahead\src" set "filename=httpd" )
		echo -I"!absolute_path!" >> %pclint_filelist_dir%\!filename!_inc.lnt
		:: 保存文件夹名称
		set filename_tmp=!filename!
		set report_filename=!report_filename! !filename_tmp!
		
		:: 将各自目录下的所有包含 .h 文件的绝对路径列举出
		cd !absolute_path!
		for /d /r %%z in (*) do (
			if exist "%%z\*.h" (
				echo -I"%%z" >> %pclint_filelist_dir%\!filename!_inc.lnt
			)
		)
		if "!filename!" == "httpd" ( set absolute_path=%target_directory%\%%u\cgi )
		:: 将目录下的所有 .c 列举出，放到 Pclint_FileList 文件夹下
		dir !absolute_path! /s /b | findstr /e \.[c] > %pclint_filelist_dir%\!filename!_src.lnt
	) else (
		echo.
		echo --------------------------------------------------------------------------------
		echo ---[ ^(%%u^)File you set is not exist, Please modify the Settings. ]---
		echo --------------------------------------------------------------------------------
	)
)
goto :eof

rem ==========================================================================================
rem                             函数 :Gets_platform_header_file_path
rem ==========================================================================================

:Gets_platform_header_file_path
cd /d %~dp0
:: 获取 release 头文件路径
echo. >> %pclint_filelist_dir%\inc_platform.lnt
if exist %pclint_include%\%release_package%\include (
	echo -I"%pclint_include%\%release_package%\include" >> %pclint_filelist_dir%\inc_platform.lnt
	cd %pclint_include%\%release_package%\include
	for /d /r %%z in (*) do (
		if exist "%%z\*.h" (
			echo -I"%%z" >> %pclint_filelist_dir%\inc_platform.lnt
		)
	)
) else (
	echo ^(  %pclint_include%\%release_package%\include ^)File you set is not exist, Please modify the Settings.
)
cd /d %~dp0
:: 获取 wifi_release 头文件路径
if exist %pclint_include%\%wifi_release_package%\include (
	echo -I"%pclint_include%\%wifi_release_package%\include" >> %pclint_filelist_dir%\inc_platform.lnt
	cd %pclint_include%\%wifi_release_package%\include
	for /d /r %%z in (*) do (
		if exist "%%z\*.h" (
			echo -I"%%z" >> %pclint_filelist_dir%\inc_platform.lnt
		)
	)
) else (
	echo ^( %pclint_include%\%wifi_release_package%\include ^)File you set is not exist, Please modify the Settings.
)
:: 回到本文件的路径下，以防以下使用相对于本文件为相对路径的错误
cd /d %~dp0&goto :eof

rem ==========================================================================================
rem                             函数 :Gets_common_header_file_path
rem ==========================================================================================

:Gets_common_header_file_path
if exist %prod_common_path% (
	cd %prod_common_path%
	echo -I"%prod_common_path%" >> %pclint_filelist_dir%\inc_prod_common.lnt
	for /d /r %%z in (*) do (
		if exist "%%z\*.h" (
			echo -I"%%z" >> %pclint_filelist_dir%\inc_prod_common.lnt
		)
	)
	cd /d %~dp0
) else (
	echo ^(%prod_common_path%^)File you set is not exist, Please modify the Settings.
)

:: 引用其他文件夹下的头文件路径
echo -I"%pclint_include%\include" >> %pclint_filelist_dir%\inc_prod_common.lnt
goto :eof

rem ==========================================================================================
rem                             函数 :get_error_warning
rem ==========================================================================================

:get_error_warning
for /f "tokens=*" %%i in ('find /c "<code-type>Error</code-type>" %1') do set error_info=%%i
set value=!error_info: =!
set value=!value:%1:=!
set /a %2=%value%

for /f "tokens=*" %%i in ('find /c "<code-type>Warning</code-type>" %1') do set error_info=%%i
set value=!error_info: =!
set value=!value:%1:=!
set /a %3=%value%
goto :EOF

rem ==========================================================================================
rem                                    函数 :time_used
rem ==========================================================================================

:time_used
@echo off&setlocal&set /a n=0
for /f "tokens=1-8 delims=.: " %%a in ("%~1:%~2") do (
set /a n+=10%%a%%100*360000+10%%b%%100*6000+10%%c%%100*100+10%%d%%100
set /a n-=10%%e%%100*360000+10%%f%%100*6000+10%%g%%100*100+10%%h%%100)
set /a s=n/360000,n=n%%360000,f=n/6000,n=n%%6000,m=n/100,n=n%%100
set "used_time=%f% min %m% sec %n% usec"
endlocal&set %~3=%used_time:-=%&goto :EOF

rem ==========================================================================================
rem                                   函数 :Undetected_list
rem ==========================================================================================

:Undetected_list
cd %pclint_filelist_dir%
if exist %pclint_rules_dir%\set_ignore_check_cFiles.lnt (
	for /f "delims=*" %%i in ( %pclint_rules_dir%\set_ignore_check_cFiles.lnt ) do (
:: 将命令行转义
		set cmd=%%i&set cmd=!cmd:\=\\!&set cmd=!cmd:.=\.!
:: 删除含有该 .c 的一行
		for %%f in (*_src.lnt) do (
			findstr /v "!cmd!" %%f > tmp.lnt
			del %%f
			ren tmp.lnt %%f
		)
	)
)
cd /d %~dp0
goto :eof

rem ==========================================================================================
rem                             函数 :xcopy_header_to_include_file
rem ==========================================================================================

:xcopy_header_to_include_file
if exist %pclint_rules_dir%\set_extern_header_path.lnt (
	for /f "delims=*" %%i in ( %pclint_rules_dir%\set_extern_header_path.lnt ) do (
		if exist %UGW6_0%\%%i (
			if not exist %pclint_include%\include ( md %pclint_include%\include )
			xcopy %UGW6_0%\%%i %pclint_include%\include /y >nul 2>nul
		)
	)
)
goto :eof

rem ==========================================================================================
rem                             函数 :set_library_path
rem ==========================================================================================

:set_library_path
echo ^-wlib^(0^) >> %pclint_rules_dir%\public_lib.lnt
if exist %pclint_rules_dir%\set_library_path.lnt (
	for /f "delims=*" %%i in ( %pclint_rules_dir%\set_library_path.lnt ) do (
		echo ^+libdir^(%UGW6_0%\%%i^) >> %pclint_rules_dir%\public_lib.lnt
	)
)
goto :eof