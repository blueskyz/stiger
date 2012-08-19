/**
 * @file   st_tools.c
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Thu May 24 15:09:47 2012
 * 
 * @brief  stiger tools, 1. create darts word dictionary, 2. load darts word dictionary
 *			3. test split word, darts word
 * 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "st_utils.h"
#include "st_darray.h"
#include "st_darts.h"

static st_darts* g_s_pDarts = NULL;

unsigned int findToken(st_darts* handler, const char* str, unsigned int uLen)
{
  st_darts_state* dState = stDartsStateNew(handler);
  if (NULL == dState){
    stErr("create darts state fail.");
  }

  for (int i = 0; i < uLen; ++i){
    uint16_t iCode = stUTF8Decode((BYTE**)&str);
    if (iCode > 0 ){
      stLog("iCode=%u", iCode);
      ST_DARTS_STATE_SET_KEY(dState, iCode);
      int nFind = stDartsFindNext(handler, dState);
      if (nFind < 0){
	stErr("can't find icode=%u", iCode);
	break;
      }
      else if (ST_DARTS_STATE_END(dState)){
	break;
      }
      else if (ST_DARTS_STATE_HAS_VALUE(dState)){
	stLog("dState=%d, wordId=%d",
	      ST_DARTS_STATE(dState),
	      ST_DARTS_STATE_VALUE(dState));
      }
    }
    else if (iCode == 0){
      // stDartsStateReset(handler, dState);
      return 0;
    }
  }
  return 0;
}

/*
#include <map>
#include <string>
using namespace std;
*/
int loadDictFromWordList(st_darts* handler, char* dicFile, unsigned int nWordCount)
{
  st_timer stTimerType = ST_TIMER_SEC;
  long long loadDicBeginTime = stTimer(stTimerType);

  FILE* fdic = fopen(dicFile, "r");
  unsigned int wordId; 
  BYTE word[MAX_PATH];
  uint16_t iCodeArr[MAX_PATH];
  stLog("load dictionary from word list file %s", dicFile);
  // fscanf(fdic, "%s\n", word);
  // stLog("title: %s\n\n", word);
  int i = 0;

  float f1 = 0.0;
  float f2 = 0.0;
  // char type[MAX_PATH] = { 0 };
  int type;
  // map<string, uint32_t> testMap;
  for ( ; i < nWordCount; ++i){
    if (feof(fdic)){
      stLog("read dict end.");
      break;
    }
    // fscanf(fdic, "%d %s\n", &wordId, word);
    fscanf(fdic, "%d:%s %f %f %d\n", &wordId, word, &f1, &f2, &type);
    // fscanf(fdic, "%d:%s %s\n", &wordId, word, word);
    // stLog("wordId=%d, f1=%f, f2=%f, type=%d", wordId, f1, f2, type);
    stLog("wordId=%d, %s", wordId, word);
    BYTE* pBuf = word;
    int nLen = strlen((char*)word);
    int n = 0;
    while(pBuf < pBuf + nLen && n < 20){
      uint16_t iCode = stUTF8Decode(&pBuf);
      if (iCode == 0) {
	stDebug("");
	break;
      }
      // add code point
      if (iCode >0){
	iCodeArr[n] = iCode;
	++n;
      }
    }
    stDebug("first code=%d, len=%d, wordId=%u, word=%s", iCodeArr[0], n, wordId, word);
    // testMap[(char*)word] = wordId;
    int nRet = stDartsPut(handler, iCodeArr, n, wordId);
    if (nRet < 0){
      stErr("Put fail!!!");
      exit(-1);
      break;
    }
  }
  fclose(fdic);

  // (sizeof(A) + sizeof(B) + ELEMENT_OVERHEAD) * N + CONTAINER_OVERHEAD
  // int nSize = (sizeof(string) + sizeof(uint32_t) + sizeof(_Rb_tree_node_base)) * testMap.size();
  // stDebug("test map memory %d",  nSize);
  long long loadDicEndTime = stTimer(stTimerType);
  stLog("load word dictionary wordcount=%d, time=%lld\n",
	i,
	loadDicEndTime - loadDicBeginTime);

  return i;
}


void testMode(st_darts* pDarts, char* encode)
{
  st_timer stTimerType = ST_TIMER_MICRO_SEC;
  long long queryBeginTime = stTimer(stTimerType);
  // char str[] = "阿弟";
  //char str[] = "白花花";
  // char str[] = "保护关税";
  // char str[] = "不得不说";
  char str[] = "不";
  stDebug("str=%s, len=%d", str, strlen(str));
  findToken(pDarts, str, sizeof(str));
  long long queryEndTime = stTimer(stTimerType);
  stLog("query word=%s, len=%d, time=%lld\n",
	str, sizeof(str), queryEndTime - queryBeginTime);


  char input[MAX_PATH];
  char output[MAX_PATH];
  //memset(input, 0, sizeof(input));
  //memset(output, 0, sizeof(output));
  printf("Plz input # ");
  fflush(stdout);
  while (fgets(input, MAX_PATH, stdin)){
    input[strlen(input)-1] = '\0';
    stDebug("input=%s, len=%d", input, strlen(input));
    stConvertCode(encode, "utf-8", input, strlen(input), output, MAX_PATH);
    stDebug("outbuf=%s, len=%d", output, strlen(output));
    findToken(pDarts, output, strlen(output));

    printf("Plz input # ");
    fflush(stdout);
  }
}


void testModeCutWord(st_darts* pDarts, char* encode)
{
  st_timer stTimerType = ST_TIMER_MICRO_SEC;
  char input[MAX_PATH];
  char output[MAX_PATH];
  uint32_t wordId[1024] = { 0 };
  uint32_t wordPos[1024] = { 0 };
  char outWord[MAX_PATH];
  //memset(input, 0, sizeof(input));
  //memset(output, 0, sizeof(output));
  st_darts_state* dState = stDartsStateNew(pDarts);

  printf("Plz input # ");
  fflush(stdout);
  while (fgets(input, MAX_PATH, stdin)){
    if (strlen(input)-1 == 0){
      printf("Plz input # ");
      fflush(stdout);
      continue;
    }
    input[strlen(input)-1] = '\0';
    stLog("input=%s, len=%d", input, strlen(input));
    stConvertCode(encode, "utf-8", input, strlen(input), output, MAX_PATH);
    uint32_t uLen = strlen(output);
    stLog("outbuf=%s, len=%u", output, uLen);

    long long queryBeginTime = stTimer(stTimerType);
    stCutWord(pDarts, dState, output, wordId, wordPos, &uLen, 10);
    long long queryEndTime = stTimer(stTimerType);
    stLog("result=%u, cost time=%lldus", uLen, queryEndTime - queryBeginTime);

    if (uLen > 0){
      char* begin = output;
      char* end = output;
      char* off = output;
      int offsetPos = 0;
      for (int i = uLen - 1; i >= 0; --i){
	      int nStep = wordPos[i] - offsetPos;
	      while(nStep--){
		      if (0 == stUTF8Decode((BYTE**)&off)){
			      break;
		      }
	      }
	offsetPos = wordPos[i];
	if (i == uLen - 1){
	  begin = off;
	}
	else {
	  end = off;
	  memcpy(outWord, begin, end - begin);
	  outWord[end-begin] = '\0';
	  stLog("query i=%d, wordId=%d, pos=%u, word=%s",
		(uLen - 1) - (1 + i), wordId[i + 1], wordPos[i + 1], outWord);
	  begin = end;
	}
      }
      memcpy(outWord, begin, output + MAX_PATH - begin);
      outWord[output + MAX_PATH - begin] = '\0';
      stLog("query i=%d, wordId=%d, pos=%u, word=%s",
	    (uLen - 1), wordId[0], wordPos[0], outWord);
    }
    printf("Plz input # ");
    fflush(stdout);
  }
}

void testModeCutWordForByte(st_darts* pDarts, char* encode)
{
  st_timer stTimerType = ST_TIMER_MICRO_SEC;
  char input[MAX_PATH];
  char output[MAX_PATH];
  const char* word[1024] = { 0 };
  uint32_t wordLen[1024] = { 0 };
  char outWord[MAX_PATH];
  //memset(input, 0, sizeof(input));
  //memset(output, 0, sizeof(output));
  st_darts_state* dState = stDartsStateNew(pDarts);

  printf("Plz input # ");
  fflush(stdout);
  while (fgets(input, MAX_PATH, stdin)){
    if (strlen(input)-1 == 0){
      printf("Plz input # ");
      fflush(stdout);
      continue;
    }
    input[strlen(input)-1] = '\0';
    stLog("input=%s, len=%d", input, strlen(input));
    stConvertCode(encode, "utf-8", input, strlen(input), output, MAX_PATH);
    uint32_t uLen = strlen(output);
    stLog("outbuf=%s, len=%u", output, uLen);

    long long queryBeginTime = stTimer(stTimerType);
    stCutWordByte(pDarts, dState, output, word, wordLen, &uLen, 10);
    long long queryEndTime = stTimer(stTimerType);
    stLog("result=%u, cost time=%lldus", uLen, queryEndTime - queryBeginTime);

    for (int i = uLen - 1; i >= 0; --i){
      memcpy(outWord, word[i], wordLen[i]);
      outWord[wordLen[i]] = '\0';
      stLog("query i=%d, word=%s, len=%u",
	    (uLen - 1 - i), outWord, wordLen[i]);
    }
    printf("Plz input # ");
    fflush(stdout);
  }
}

void usage()
{
  printf("usage: ./st_tools <dictMode> <task> [option]\n"
	 "  dictMode:\n"
	 "	-s file: load word from word list file\n"
	 "	-l file: load dictionary from darts file\n"
	 "  task:\n"
	 "	-c file: create darts file from memory\n"
	 "	-t mode: test mode [0. test word  1. test cut word 2. test cut word for byte]\n"
	 "	-h: help\n"
	 "  option:\n"
	 "	-n: <dictMode = -s> word count, default 10\n"
	 "	-u: encode: 0: utf-8, 1: gbk, default 1\n"
	 "	-m unit: use memory rate, unit >= 1000 \n"
	 "\n");
}

int main(int argc, char *argv[])
{
  int nWordCount = 10;
  int bModeWordList = 1;
  int bModeTest = 0;
  int nModeTest = 0;
  int bModeCreateDarts = 0;
  char dictFile[MAX_PATH] = { '\0' };
  char dartsFile[MAX_PATH] = { '\0' };
  int nEncodeIndex = 1;
  int32_t nStatisticUnit = 0;
  char* encode[] = {"utf-8", "gbk"};

  if (argc == 1){
      usage();
      return 0;
  }
  int c = 0;
  while ((c = getopt(argc, argv, "s:l:c:t:n:u:m:h")) != -1){
    switch(c){
    case 'h':
      usage();
      return 0;
    case 'l':
      bModeWordList = 0;
    case 's':
      if (strlen(optarg) >= MAX_PATH){
	stErr("dictionary mode -%c=%s, path too long, >%d", (char)c, optarg, MAX_PATH);
	return -1;
      }
      memcpy(dictFile, optarg, strlen(optarg));
      dictFile[strlen(optarg)] = '\0';
      stLog("**  dictionary mode -%c=%s", (char)c, optarg);
      break;
    case 'c':
      if (strlen(optarg) >= MAX_PATH){
	stErr("dictionary mode -%c=%s, path too long, >%d", (char)c, optarg, MAX_PATH);
	return -1;
      }
      memcpy(dartsFile, optarg, strlen(optarg));
      dartsFile[strlen(optarg)] = '\0';
      stLog("**  param darts file %s", dartsFile);
      bModeCreateDarts = 1;
      break;
    case 't':
      bModeTest = 1;
      nModeTest = atoi(optarg);
      if (nModeTest != 0 
          && nModeTest != 1
          && nModeTest != 2){
	stErr("error test mode %d", nModeTest);
	usage();
	return -1;
      }
      stLog("**  nModeTest %d", nModeTest);
      break;
    case 'n':
      nWordCount = atoi(optarg);
      stLog("**  Load word count %d", nWordCount);
      break;
    case 'u':
      nEncodeIndex = atoi(optarg);
      if (nEncodeIndex < 0 && nEncodeIndex >= sizeof(encode)){
	stErr("error encode type %d", nEncodeIndex);
	usage();
	return -1;
      }
      break;
    case 'm':
      nStatisticUnit = atoi(optarg);
      break;
    default:
      usage();
      return -1;
    }
  }
  stLog("**  encode %s", encode[nEncodeIndex]);

  // load Dic
  if (strlen(dictFile) <= 0){
    stErr("you must select dictMode and file path!!!");
    return -1;
  }
  if (bModeWordList){
    // g_s_pDarts = stDartsNew(1024*1024*3);
    g_s_pDarts = stDartsNew(1024*640);
    loadDictFromWordList(g_s_pDarts, dictFile, nWordCount);
  }
  else {
    g_s_pDarts = stDartsLoad(dictFile);
  }

  if (bModeCreateDarts
      && (0 != stDartsSave(g_s_pDarts, dartsFile))){
    return -1;
  }

  if (nStatisticUnit != 0){
    if (nStatisticUnit < 1000){
      stErr("nStatisticUnit=%d, < 1000", nStatisticUnit);
      return -1;
    }
    stLog("--  statistics mem with unit=%d", nStatisticUnit);
    stDartsStatistics(g_s_pDarts, nStatisticUnit);
    stLog("--");
  }
  
  // test mode
  if (bModeTest){
    stDebug("nModeTest=%d, %d", nModeTest, sizeof(encode));
    if (nModeTest == 0)
      testMode(g_s_pDarts, encode[nEncodeIndex]);
    if (nModeTest == 1)
      testModeCutWord(g_s_pDarts, encode[nEncodeIndex]);
    if (nModeTest == 2)
      testModeCutWordForByte(g_s_pDarts, encode[nEncodeIndex]);
    else {
      usage();
      return -1;
    }
    
  }
  
  return 0;
}
