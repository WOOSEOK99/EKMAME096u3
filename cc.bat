@echo off
path=
rem ---------------------------------------------
rem 사용하는 개발 환경에 맞추어서 작성하여 주세요
rem ---------------------------------------------

set MINGW_HOME=d:\MinGW

rem ---------------------------------------
rem mingw32와 각종 라이브러리의 경로를 설정
rem ---------------------------------------

set INCLUDE=%MINGW_HOME%\include;
set PATH=%MINGW_HOME%\bin;%PATH%

rem --------------------------------------
rem Win9x or WinNT or WinXP 에서 컴파일
rem --------------------------------------
cls
touch src\sound\psx.c
touch src\vidhrdw\psx.c
make WINUI=1 I686=1 USE_PSXPLUGIN=1
