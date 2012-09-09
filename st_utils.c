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
#include <ctype.h>
#include <math.h>
#include <iconv.h>

#include <sys/time.h>
#include <sys/stat.h>
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
  int log = open("/var/log/mysql/mysql.tft.log", 
		  O_WRONLY | O_APPEND | O_CREAT,
		  S_IRWXU);
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


// 特殊符号的内存整数值表，(适用x86)不可移植
// symbol variable | unicode | comment
#define s_nSymbol_0 12298 // 《
#define s_nSymbol_1 12299 // 》
#define s_nSymbol_2 65292 // ，
#define s_nSymbol_3 12290 // 。
#define s_nSymbol_4 65509 // ￥
#define s_nSymbol_5 8216 // ‘
#define s_nSymbol_6 8220 // “
#define s_nSymbol_7 65307 // ；
#define s_nSymbol_8 65306 // ：
#define s_nSymbol_9 65371 // ｛
#define s_nSymbol_10 65373 // ｝
#define s_nSymbol_11 12304 // 【
#define s_nSymbol_12 12305 // 】
#define s_nSymbol_13 65288 // （
#define s_nSymbol_14 65289 // ）
#define s_nSymbol_15 44 // ,
#define s_nSymbol_16 46 // .
#define s_nSymbol_17 32 //
#define s_nSymbol_18 63 // ?
#define s_nSymbol_19 39 // '
#define s_nSymbol_20 34 // "
#define s_nSymbol_21 33 // !
#define s_nSymbol_22 58 // :
#define s_nSymbol_23 59 // ;
#define s_nSymbol_24 60 // <
#define s_nSymbol_25 62 // >

int stPrintFilterSymbol()
{
  // 打印符号表 中文15,英文11
  const uint32_t symbolLen = 26;
  static const char s_sSymbol[26][4] = {
	{"《"}, {"》"}, {"，"}, {"。"},
	{"￥"}, {"‘"}, {"“"}, {"；"},
	{"："}, {"｛"}, {"｝"}, {"【"},
	{"】"}, {"（"}, {"）"}, 
	{","}, {"."}, {" "}, {"?"}, 
	{"\'"}, {"\""}, {"!"}, {":"}, 
	{";"}, {"<"}, {">"}
  };
  stLog("// symbol variable | unicode | comment");
  for(int i = 0 ; i < symbolLen ; ++i){
	BYTE* p = (BYTE*)s_sSymbol[i];
	int nCode = stUTF8Decode(&p);
	if (nCode == 0){
	  break;
	}
	else if (nCode == -1){
	  return -1;
	}
	else {
	  stLog("#define s_nSymbol_%d %d // %s", i, nCode, s_sSymbol[i]);
	}
  }
  return 0;
}

// -1:invalid split[,. ...] 0: english 1:zh and other
int stFilterSymbol(int nCode)
{
  // 英文
  if (nCode < 128){
	if (isalnum(nCode)){
	  return 0;
	}
	else{
	  return -1;
	}
  }

  // 中英文分隔符判断, 与上面的判断有重复
  switch(nCode){
	case s_nSymbol_0:
	case s_nSymbol_1:
	case s_nSymbol_2:
	case s_nSymbol_3:
	case s_nSymbol_4:
	case s_nSymbol_5:
	case s_nSymbol_6:
	case s_nSymbol_7:
	case s_nSymbol_8:
	case s_nSymbol_9:
	case s_nSymbol_10:
	case s_nSymbol_11:
	case s_nSymbol_12:
	case s_nSymbol_13:
	case s_nSymbol_14:
	case s_nSymbol_15:
	case s_nSymbol_16:
	case s_nSymbol_17:
	case s_nSymbol_18:
	case s_nSymbol_19:
	case s_nSymbol_20:
	case s_nSymbol_21:
	case s_nSymbol_22:
	case s_nSymbol_23:
	case s_nSymbol_24:
	case s_nSymbol_25:
	  return -1;
  }

  return 1;
}

void stToLower(char* pStr, uint32_t uLen)
{
  while(uLen--){
	*pStr++ = tolower(*pStr);
  }
}
