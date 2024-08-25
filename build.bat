@ECHO OFF

REM WINDOWS BATCH BUILD
REM ARG1 (optional): CONFIG

SET CURRENTCFG=%1

IF "%CURRENTCFG%"=="" SET CURRENTCFG=Debug

IF NOT "%CURRENTCFG%"=="Debug" IF NOT "%CURRENTCFG%"=="Release" (
	ECHO Unknown config %CURRENTCFG%
	EXIT /B
)

ECHO Building %CURRENTCFG%

cmake .
cmake --build . -j 6 --parallel 6 --target rungame --config %CURRENTCFG%