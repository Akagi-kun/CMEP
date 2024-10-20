@ECHO OFF

REM WINDOWS BATCH BUILD
REM ARG1 (optional): CONFIG

SET CURRENTCFG=%1

REM Default to 'Debug'
IF "%CURRENTCFG%"=="" SET CURRENTCFG=Debug

IF NOT "%CURRENTCFG%"=="Debug" IF NOT "%CURRENTCFG%"=="Release" (
	ECHO Unknown config %CURRENTCFG%
	EXIT /B
)

ECHO Building %CURRENTCFG%

cmake -DCMAKE_BUILD_TYPE=%CURRENTCFG% . || ECHO CMake configure failed, exitting && EXIT /B
cmake --build . --parallel 6 --target rungame --config %CURRENTCFG%