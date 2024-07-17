@echo off
setlocal

set ENGLISH_TS_OLD=en_US.ts
set CHINESE_TS_OLD=zh_CN.ts

rem 生成 qm 文件
lrelease %ENGLISH_TS_OLD% -qm en_US.qm
lrelease %CHINESE_TS_OLD% -qm zh_CN.qm