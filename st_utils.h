/**
 * @file   st_utils.h
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Fri May 18 16:05:42 2012
 * 
 * @brief  utility tools
 * 
 * 
 */

#ifndef _ST_UTILS_H_
#define _ST_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <fcntl.h>

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define stDetail(logMsgFmt, args...)				\
  do{									\
    char logBuf[MAX_PATH];							\
    int size = snprintf(logBuf, MAX_PATH-1, logMsgFmt "\n", ##args); \
    printf("%s", logBuf);						\
    stLogToFile(logBuf, size);						\
    fflush(stdout);							\
  }while(0)

#define stLog(logMsgFmt, args...) stDetail("[LOG]" logMsgFmt, ##args)
#define stErr(logMsgFmt, args...) stDetail("[ERROR]" logMsgFmt, ##args)

#ifndef DEBUG
#define stDebug(logMsgFmt, args...)
#else
#define stDebug(logMsgFmt, args...) stDetail("[DEBUG]", logMsgFmt, ##args)
#endif

#define min(x,y) ((x)<(y)?(x):(y))

// code method
int stUTF8Decode(BYTE** pBuf);
int stConvertCode(const char* srcCode,
		  const char* destCode,
		  char* src,
		  size_t srcLen,
		  char* dest,
		  size_t destLen);

int stPrintFilterSymbol();
int stFilterSymbol(int nCode);
void stToLower(char* pStr, uint32_t uLen);

// log file
void stLogToFile(const char* content, unsigned int uLen);

// time method
typedef enum { ST_TIMER_SEC, ST_TIMER_MILLI_SEC, ST_TIMER_MICRO_SEC } st_timer;
long long stTimer(st_timer stTimeType);

#endif /* _ST_UTILS_H_ */
