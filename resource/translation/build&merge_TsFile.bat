@echo off
setlocal

rem 设置目录变量
set SOURCE_DIR=..\..\src
set ENGLISH_TS_NEW=new_en.ts
set ENGLISH_TS_OLD=en_US.ts
set CHINESE_TS_NEW=new_cn.ts
set CHINESE_TS_OLD=zh_CN.ts

rem 使用 lupdate 生成新的英文 .ts 文件
lupdate %SOURCE_DIR% -ts %ENGLISH_TS_NEW%

rem 使用 lupdate 生成新的中文 .ts 文件
lupdate %SOURCE_DIR% -ts %CHINESE_TS_NEW% -target-language zh_CN

rem 如果旧的英文 .ts 文件存在，则合并新的 .ts 文件和旧的 .ts 文件
if exist %ENGLISH_TS_OLD% (
    lconvert -i %ENGLISH_TS_NEW% -i %ENGLISH_TS_OLD% -o merged_en.ts
    del %ENGLISH_TS_NEW%
    move /Y merged_en.ts %ENGLISH_TS_OLD%
    echo Merged and renamed English .ts file.
) else (
    echo English .ts file doesn't exist.
    move /Y new_en.ts %ENGLISH_TS_OLD%
)

rem 如果旧的中文 .ts 文件存在，则合并新的 .ts 文件和旧的 .ts 文件
if exist %CHINESE_TS_OLD% (
    lconvert -i %CHINESE_TS_NEW% -i %CHINESE_TS_OLD% -o merged_cn.ts
    del %CHINESE_TS_NEW%
    move /Y merged_cn.ts %CHINESE_TS_OLD%
    echo Merged and renamed Chinese .ts file.
) else (
    echo Chinese .ts file doesn't exist.
    move /Y new_cn.ts %CHINESE_TS_OLD%
)

echo TS file generation completed.