/**
 * @file   st_utils.c
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Fri May 18 16:10:17 2012
 * 
 * @brief  
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <iconv.h>

#include <sys/time.h>
#include <unistd.h>

#include "st_utils.h"

// ------------------------------------------------------------
// code method
int stUTF8Decode(BYTE** ppBuf)
{
  BYTE* pBuf = *ppBuf;
  BYTE v = *pBuf;
  if ( !v )
    return 0;
  pBuf++;

  // check for 7-bit case
  if ( v<128 ){
    *ppBuf = pBuf;
    return v;
  }

  // get number of bytes
  int iBytes = 0;
  while ( v & 0x80 ){
    iBytes++;
    v <<= 1;
  }

  // check for valid number of bytes
  if ( iBytes<2 || iBytes>4 )
    return -1;

  int iCode = ( v >> iBytes );
  iBytes--;
  do{
    if ( !(*pBuf) )
      return 0; // unexpected eof

    if ( ((*pBuf) & 0xC0)!=0x80 )
      return -1; // invalid code

    iCode = ( iCode<<6 ) + ( (*pBuf) & 0x3F );
    iBytes--;
    ++pBuf;
    *ppBuf = pBuf;
  } while ( iBytes );

  // all good
  return iCode;
}

int stConvertCode(const char* srcCode,
		  const char* destCode,
		  char* src,
		  size_t srcLen,
		  char* dest,
		  size_t destLen)
{
  if (NULL == srcCode || NULL == destCode){
    return -1;
  }
  iconv_t handle = iconv_open(destCode, srcCode);
  if ((iconv_t)-1 == handle){
    return -1;
  }
  memset(dest, 0, destLen);
  size_t size = iconv(handle, &src, &srcLen, &dest, &destLen);
  if ((size_t)-1 == size){
    return -1;
  }
  iconv_close(handle);
  return size;
}

void unicodeToUtf16(int16_t* iCode, unsigned int uSize)
{
  for (int i = 0; i < uSize; ++i){
    char* tmp = (char*)(iCode+i);
    char tmpC = tmp[0];
    tmp[0] = tmp[1];
    tmp[1] = tmpC;
  }
}


void stLogToFile(const char* content, unsigned int uLen)
{
  int log = open("/var/log/mysql.tft.log", O_WRONLY, O_APPEND | O_CREAT);
  write(log, content, uLen);
  close(log);
}

// ------------------------------------------------------------
// timer
long long stTimer(st_timer stTimeType)
{
  if (stTimeType < ST_TIMER_SEC || stTimeType > ST_TIMER_MICRO_SEC){
    return -1;
  }
  struct timeval tVal;
  gettimeofday(&tVal, NULL);
  return (long long)tVal.tv_sec * pow(1000, stTimeType)
    + tVal.tv_usec / pow(1000, (ST_TIMER_MICRO_SEC - stTimeType));
}
