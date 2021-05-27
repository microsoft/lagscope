@echo off
REM Wrapper script around cmake commands

goto :init

:usage
	echo Usage: .\%~n0%~x0 [build] [install] [clean] [distclean]
	echo build     : Build or rebuild lagscope binary
	echo install   : Copy lagscope binary to install folder
	echo clean     : make clean
	echo distclean : Remove build folder
	goto :end

:init
	set args=0
	for %%x in (%*) do set /a args+=1
	if "%args%" neq "1" goto :usage


set returndir=%cd%
if "%1"=="build" goto :build
if "%1"=="install" goto :install
if "%1"=="clean" goto :clean
if "%1"=="distclean" goto :distclean
goto :usage

:build
	mkdir build
	cd build
	cmake ../src
	cmake --build . --config Release
	goto :end

:install
	set returndir=%cd%
	cd build
	cmake --build . --target install
	goto :end

:clean
	set returndir=%cd%
	cd build
	cmake --build . --target clean
	goto :end

:distclean
	rd /s /q build

:end
	cd %returndir%
