@echo off

for /f %%i in ('git rev-parse --short HEAD') do set commit_id=%%i

echo #ifndef __CONFIG_H__                > include\config.h
echo #define __CONFIG_H__               >> include\config.h
echo #define XXXX_REV   "%commit_id%"   >> include\config.h

echo #endif  /* __CONFIG_H__ */         >> include\config.h
