@ECHO OFF

REM WINDOWS BATCH BUILD
REM ARG1 (optional): CONFIG

SET CURRENTCFG=%1

IF "%CURRENTCFG%"=="" SET CURRENTCFG=Debug

ECHO Building %CURRENTCFG%

cmake .
cmake --build . -j --parallel --target rungame --config %CURRENTCFG%