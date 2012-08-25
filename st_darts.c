/**
 * @file   st_darts.c
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:52:17 2012
 * 
 * @brief  double array trie implement
 * 
 * 
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "st_utils.h"
#include "st_darray.h"
#include "st_darts.h"

static uint32_t g_s_uDartsMagicBase = 0x000f;
#define g_s_subSize 1024 << 7

// data struct
struct st_darts
{
  uint32_t uMagic;
  uint32_t arrayLen;
  uint32_t uLimit;
  uint32_t uRoot;
  st_darray* base;
  st_darray* check;
  uint32_t uMmapSize;
  int32_t subCodeSize;
  int32_t subCode[g_s_subSize];
};

typedef struct st_darts_unit
{
  int base;
  unsigned int uValue;
  // uint16_t uLimit;
}st_darts_unit;

// method
int stDartsFindBase(st_darts* handler, int s, uint16_t uCode)
{
  st_darts_unit* pBase = (st_darts_unit*)stDArrayPointer(handler->base);
  int* pCheck = (int*)stDArrayPointer(handler->check);

  int oldBase = abs(pBase[s].base);
  int n = 0;
  int* subCode = handler->subCode;
  // uint16_t uLimit = pBase[s].uLimit;
  // for (int i = oldBase + 1 ; i < handler->arrayLen ; ++i){
  for (int i = oldBase + 1 ; i <= handler->uLimit ; ++i){
    if (pCheck[i] == s && n < g_s_subSize){
      subCode[n++] = i - oldBase;
      // --uLimit;
    }
    /*
    if (uLimit <= 0){
      break;
    }
    */
  }
  subCode[n++] = uCode;
  stDebug("handler->uLimit=%u, subCodeCount=%d", handler->uLimit, n);
  int minCode = subCode[0] < uCode ? subCode[0] : uCode;
  int i = 0;
  for (i = -pCheck[0] ; i != 0 ; i = -pCheck[i]) {
    if (i >= handler->arrayLen)
      return -1; // memory ???
    // if (i > oldBase && i > minCode)
    if (i > minCode && i >oldBase)
      break;
  }
  stDebug("pCheck oldBase=%d, i=%d, n=%d, minCode=%d", oldBase, i, n, minCode);
  for ( ; i != 0 && i < handler->arrayLen ; i = -pCheck[i]){
    // stDebug("-- i=%d", i);
    int j = 0;
    for ( ; j < n ; ++j ){
      int tmp = i + subCode[j] - minCode;
      if (pCheck[tmp] >= 0 || pBase[tmp].base >= 0)
		  break;
    }
    // stDebug("-- pCheck i=%d j=%d", i, j);
    if (j == n){
      handler->subCodeSize = n - 1;
      return i - minCode;
    }
  }
  return -1;
}

int stDartsRelocate(st_darts* handler, int s, unsigned int uNewBase)
{
  st_darts_unit* pBase = (st_darts_unit*)stDArrayPointer(handler->base);
  int* pCheck = (int*)stDArrayPointer(handler->check);

  unsigned int oldBase = abs(pBase[s].base);
  for (int i = handler->subCodeSize - 1 ; i >= 0 ; --i){
    int news = uNewBase + handler->subCode[i];
    int olds = oldBase + handler->subCode[i];

    /*
    stLog("i=%d, new s=%d, old s=%d, wordId=%u, baseValue=%d, uCode=%u",
	  i, news, olds, pBase[olds].uValue, pBase[olds].base, handler->subCode[i]);
    */

    // malloc node
    pBase[-pCheck[news]].base = pBase[news].base;
    pCheck[-pBase[news].base] = pCheck[news];

    pCheck[news] = s;
    memcpy(pBase + news, pBase + olds, sizeof(st_darts_unit));
    int oldi = abs(pBase[olds].base);
    for (int j = oldi + 1; j < handler->uLimit; ++j){
      if (pCheck[j] == olds)
	pCheck[j] = news;
    }

    // free node
    pCheck[olds] = -(handler->arrayLen-1);
    pCheck[-pBase[handler->arrayLen-1].base] = -olds;
    pBase[olds].base = pBase[handler->arrayLen-1].base;
    pBase[handler->arrayLen-1].base = -olds;
    if (news > handler->uLimit){
      handler->uLimit = news;
    }
  }
  pBase[s].base = (pBase[s].base < 0 ? -1 : 1) * uNewBase;

  return 0;
}

// interface
/** 
 * @brief create double array trie
 * @param baseArrayLen init darray for base and check
 * 
 * @return not null: succ, null: fail
 */
st_darts* stDartsNew(unsigned int baseArrayLen)
{
  st_darts* pdarts = (st_darts*)malloc(sizeof(st_darts));
  assert(pdarts);
  if (NULL == pdarts){
    return NULL;
  }
  pdarts->uMagic = g_s_uDartsMagicBase;
  pdarts->arrayLen = baseArrayLen;
  pdarts->base = stDArrayNew(baseArrayLen, sizeof(st_darts_unit));
  if (NULL == pdarts->base){
    free(pdarts);
    return NULL;
  }
  pdarts->check = stDArrayNew(baseArrayLen, sizeof(int));
  if (NULL == pdarts->check){
    free(pdarts->base);
    free(pdarts);
    return NULL;
  }
  stLog("create darts succ.");

  pdarts->uRoot = 0;
  pdarts->uLimit = 0;

  st_darts_unit* pBase = (st_darts_unit*)stDArrayPointer(pdarts->base);
  int* pCheck = (int*)stDArrayPointer(pdarts->check);

  // double free list
  pBase[0].base = 0;
  pCheck[0] = -1;
  for ( int i = 1 ; i < baseArrayLen ; ++i ){
    pBase[i].base = -(i - 1);
    pCheck[i] = -(i + 1);
  }
  stLog("init darts succ.");
  return pdarts;
}

/** 
 * @brief destroy double array trie
 * 
 * @param handler the pointer for st_darts
 * 
 * @return 0: succ, -1: fail
 */
int stDartsFree(st_darts* handler)
{
  if (NULL == handler || handler->uMagic < g_s_uDartsMagicBase){
    return -1;
  }
  if (NULL != handler->check){
    free(handler->check);
  }
  if (NULL != handler->base){
    free(handler->base);
  }
  free(handler);
  return 0;
}


int stDartsFreeMmap(st_darts* handler)
{
  if (NULL == handler || handler->uMagic < g_s_uDartsMagicBase){
    return -1;
  }
  if (handler->uMmapSize != 0){
	  char* pMmap = ((char*)handler->base) - 16 - 4;
	  munmap((void*)pMmap, handler->uMmapSize);
  }
  free(handler);
  return 0;
}

/** 
 * @brief save darts to file
 * 
 * @param handler pointer for st_darts
 * @param filePath the filePath
 * @see stDartsLoad()
 * 
 * @return 0: succ, -1: fail
 */
int stDartsSave(st_darts* handler, const char* filePath)
{
  if (NULL == filePath){
    stErr("save darts fail, filePath is empty.");
    return -1;
  }
  uint32_t nLen = 0;
  FILE *fp = fopen(filePath, "wb");
  if (NULL == fp){
    stErr("save darts fail, open file %s fail, %s", filePath, strerror(errno));
    return -1;
  }

  // write st_darts: uMagic, uLength, uLimit, uRoot
  if (4 != fwrite(handler, sizeof(uint32_t), 4, fp)){
    stErr("save darts fail, write uMagic uLength uLimit uRoot err.");
    goto fail;
  }

  // write base array
  nLen = stDArrayGetMemSize(handler->base);
  if (1 != fwrite(&nLen, sizeof(nLen), 1, fp)){
    stErr("save darts fail, write base darray length err.");
    goto fail;
  }
  if (nLen != fwrite(handler->base, 1, nLen, fp)){
    stErr("save darts fail, write base darray err.");
    goto fail;
  }
  stLog("save darts base data length, %u", nLen);

  // write check array
  nLen = stDArrayGetMemSize(handler->check);
  if (1 != fwrite(&nLen, sizeof(nLen), 1, fp)){
    stErr("save darts fail, write check darray length err.");
    goto fail;
  }
  if (nLen != fwrite(handler->check, 1, nLen, fp)){
    stErr("save darts fail, write check darray err.");
    goto fail;
  }
  stLog("save darts check data length, %u", nLen);

  stLog("save darts succ.");
  fclose(fp);
  return 0;

 fail:
  fclose(fp);
  return -1;
}

/** 
 * @brief load darts from file
 * 
 * @param handler
 * @see stDartsSave()
 * 
 * @return 0: succ, -1: fail
 */
st_darts* stDartsLoad(const char* filePath)
{
  stLog("load darts from %s.", filePath);
  if (NULL == filePath){
    stErr("load darts fail, filePath is empty.");
    return NULL;
  }
  FILE *fp = fopen(filePath, "rb");
  if (NULL == fp){
    stErr("load darts, open file %s fail, %s", filePath, strerror(errno));
    return NULL;
  }

  // read st_darts: uMagic, uLength, uLimit, uRoot
  uint32_t uMagic = 0;
  uint32_t uLength = 0;
  uint32_t nDArrMemSize = 0;
  st_darts* handler = NULL;
  int nRead = sizeof(uint32_t);
  if (1 != fread(&uMagic, nRead, 1, fp)){
    stErr("load darts fail, read uMagic err.");
    goto fail;
  }
  if (1 != fread(&uLength, nRead, 1, fp)){
    stErr("load darts fail, read uLength err.");
    goto fail;
  }
  handler = stDartsNew(uLength);
  handler->uMagic = uMagic;
  if (1 != fread(&(handler->uLimit), nRead, 1, fp)){
    stErr("load darts fail, read uLimit err.");
    goto fail;
  }
  if (1 != fread(&(handler->uRoot), nRead, 1, fp)){
    stErr("load darts fail, read uRoot err.");
    goto fail;
  }

  // read double array
  nRead = sizeof(uint32_t);
  if (1 != fread(&nDArrMemSize, nRead, 1, fp)){
    stErr("load darts fail, read base length err.");
    goto fail;
  }

  if (nDArrMemSize != fread(handler->base, 1, nDArrMemSize, fp)){
    if (ferror(fp)){
      stErr("load darts fail, read base content, read err.");
    }
    else if (feof(fp)){
      stErr("load darts fail, read base content, file eof.");
    }
    stErr("load darts fail, read base content err, length=%u.", nDArrMemSize);
    goto fail;
  }

  if (1 != fread(&nDArrMemSize, nRead, 1, fp)){
    stErr("load darts fail, read check length err.");
    goto fail;
  }
  if (nDArrMemSize != fread(handler->check, 1, nDArrMemSize, fp)){
    stErr("load darts fail, read check content err.");
    goto fail;
  }

  stLog("load darts succ.");
  fclose(fp);
  return handler;

 fail:
  fclose(fp);
  if (NULL == handler){
    stDartsFree(handler);
  }
  return NULL;
}

st_darts* stDartsLoadMmap(const char* filePath)
{
#ifdef __linux__
  stLog("load darts from %s, type mmap.", filePath);
  if (NULL == filePath){
    stErr("load darts fail, filePath is empty.");
    return NULL;
  }
  int fd = open(filePath, O_RDONLY, S_IRUSR);
  if (-1 == fd){
    stErr("load darts, open file %s fail, %s", filePath, strerror(errno));
    return NULL;
  }
  struct stat sb;
  if (fstat(fd, &sb) == -1){
    stErr("load darts fail, fstat");
    goto fail;
  }
  char* addr = mmap(NULL, sb.st_size, PROT_READ, MAP_LOCKED | MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED){
    stErr("load darts fail, mmap %s", strerror(errno));
    goto fail;
  }
  close(fd);

  struct st_darts* handler = (st_darts*)malloc(sizeof(st_darts));
  if (NULL == handler){
    stErr("load darts fail, new darts err.");
    goto fail;
  }
  uint32_t uOffset = 0;
  memcpy(handler, addr, sizeof(uint32_t) << 2); 
  handler->base = (struct darray*)(addr + 16 + 4);
  memcpy(&uOffset, addr + 16, sizeof(uint32_t));
  handler->check = (struct darray*)(addr + 16 + 8 + uOffset);
  return handler;

 fail:
  close(fd);
#else
  stErr("the platform is not linux, not support mmap.");
#endif
  return NULL;
}

/** 
 * @brief init search state
 * 
 * @param handler 
 * 
 * @return not null: succ/the pointer for state, null: fail
 */
st_darts_state* stDartsStateInit(st_darts* handler, 
				st_darts_state* pDartsState,
				const char* start,
				const char* end)
{
  if (NULL == pDartsState){
    return NULL;
  }
  pDartsState->uMagic = handler->uMagic;
  pDartsState->state = eds_begin;
  pDartsState->uSState = 0;
  pDartsState->start = start;
  pDartsState->end = end;
  pDartsState->uHasDecWords = 0;
  pDartsState->uHasProcWords = 0;
  pDartsState->uCurWordPos = 0;
  return pDartsState;
}

/** 
 * @brief destroy state object
 * 
 * @param handler 
 * @param state 
 * 
 * @return 
 */
int stDartsStateFree(st_darts* handler, st_darts_state* state)
{
  if (NULL == handler || NULL == state){
    return -1;
  }
  if (handler->uMagic != state->uMagic){
    return -1;
  }
  free(state);
  return 0;
}

/** 
 * @brief reset state for reuse
 *
 * @param handler
 * @param state 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsStateReset(st_darts* handler, st_darts_state* state)
{
  state->state = eds_begin;
  state->uSState = 0;
  return 0;
}

/** 
 * @brief find value, state for check
 * 
 * @param handler 
 * @param state 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsFindNext(st_darts* handler, st_darts_state* state)
{
  st_darts_unit* pBase = (st_darts_unit*)stDArrayPointer(handler->base);
  int* pCheck = (int*)stDArrayPointer(handler->check);
  unsigned int uTState = 0;
  if (state){
    int s = 0;
    if (ST_DARTS_STATE_BEGIN(state)){
      s = 0;
    }
    else if (ST_DARTS_STATE_MID(state)){
      s = state->uSState;
    }
    else {
      state->state = eds_end;
      return -1;
    }
    uTState = abs(pBase[s].base) + state->uKey;
    stDebug("s=%u, t=%u, Base[s]=%d, Check[t]=%d, Base[t]=%d",
	    s, uTState, abs(pBase[s].base), pCheck[uTState], pBase[uTState].base);
    if (pCheck[uTState] == s){
      state->uSState = uTState;
      state->state = eds_mid;
      if (pBase[uTState].base <= 0){
		  state->state |= eds_has_value;
		  state->uValue = pBase[uTState].uValue;
      }
    }
    else {
      state->state = eds_end;
	  return -1;
    }
    return 0;
  }
  return -1;
}

void stDartsStatistics(st_darts* handler, uint32_t uStep)
{
  int* pCheck = (int*)stDArrayPointer(handler->check);

  stLog("base array arraySize=%d, objSize=%d, memory=%d",
	stDArrayLen(handler->base),
	stDArrayUnitSize(handler->base),
	stDArrayGetMemSize(handler->base));
  stLog("check array arraySize=%d, objSize=%d, memory=%d",
	stDArrayLen(handler->check),
	stDArrayUnitSize(handler->check),
	stDArrayGetMemSize(handler->check));

  int nFree = 0;
  for (int i = -pCheck[0] ; i < handler->arrayLen ; i = -pCheck[i]){
    ++nFree;
  }

  int nUse = 0;
  for (int i = 0 ; i < handler->arrayLen ; ++i){
    if (pCheck[i] >= 0 )
      ++nUse;
  }
  stLog("total length=%d, free=%d, freeRate=%f, use=%d, useRate=%f",
	stDArrayLen(handler->check),
	nFree,
	(float)nFree / stDArrayLen(handler->check),
	nUse,
	(float)nUse / stDArrayLen(handler->check));

  nUse = 0;
  int nUseStep = 0;
  for (int i = 0 ; i < handler->arrayLen ; ++i){
    if (pCheck[i] >= 0 ){
      ++nUse;
      ++nUseStep;
    }
    if (i % uStep == 0 && i != 0){
      stLog("total i=%d, useStep=%d, useRateStep=%f, use=%d, useRateTotal=%f",
	    i, nUseStep, (float)nUseStep / uStep, nUse, (float)nUse / i);
      nUseStep = 0;
    }
  }
}


/** 
 * @brief add key and value to darts
 * 
 * @param handler 
 * @param pKey the key for search
 * @param uLen the key length
 * @param uValue the value for other infomation( eg: pointer for ... )
 * 
 * @return 0: succ, -1: fail
 */
int stDartsPut(st_darts* handler,
	       uint16_t* pKey,
	       unsigned int uLen,
	       unsigned int uValue)
{
  int s = 0;
  int t;
  st_darts_unit* pBase = (st_darts_unit*)stDArrayPointer(handler->base);
  int* pCheck = (int*)stDArrayPointer(handler->check);
  for (int i = 0; i < uLen; ++i){
    uint16_t uCode = pKey[i];
    t = abs(pBase[s].base) + uCode;
    if (t > handler->arrayLen){
      stDebug("Put fail .");
      return -1;
    }
    if (t > handler->uLimit){
      handler->uLimit = t;
    }

    stDebug("s=%d, t=%d, uCode=%u, value=%u, check[t]=%d, pBase[s].base=%d",
	    s, t, uCode, uValue, pCheck[t], pBase[s].base);

    if (s == pCheck[t]){
      s = t;
    }
    else if (pBase[t].base < 0 && pCheck[t] < 0){
      // get node
      pBase[-pCheck[t]].base = pBase[t].base;
      pCheck[-pBase[t].base] = pCheck[t];
      pBase[t].base = 2;
      pCheck[t] = s;
      // pBase[s].uLimit++;
      s = t;
    }
    else { // relocate
      stDebug("relocate .");
      handler->subCodeSize = 0;
      int newBase = stDartsFindBase(handler, s, uCode);
      if (newBase < 0){
	stErr("Find base failure s=%d, uCode=%u", s, uCode);
	return -1; // memory ???
      }
      else {
	stDebug("find new base s=%d, uCode=%u, newBase=%d", s, uCode, newBase);
	stDartsRelocate(handler, s, newBase);
	--i;
      }
    }
  }
  pBase[s].uValue = uValue;
  pBase[s].base = -abs(pBase[s].base);

  return 0;
}

#define ST_MAGIC_SENTENCE	1024
/** 
 * @brief automatic segmentation model
 * 
 * @param str the string length = uLen, the str length < 1024
 * @param wordIdArr wordId list, array length = 2 * uLen
 * @param posArr the word position, array length = 2 * uLen
 * @param uLen
 * @param bAsc 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsCutWord(st_darts* handler,
	      st_darts_state* dState,
	      const char* str,
	      uint32_t* wordIdArr,
	      uint32_t* posArr,
	      uint32_t* pLen,
	      uint32_t uStep /* int bAsc */ )
{
  assert(NULL != str && 0 != *pLen && 0 != uStep);
  uint32_t uCodeArr[ST_MAGIC_SENTENCE] = { 0 };
  int i = 0;
  for (i = 0; i < ST_MAGIC_SENTENCE ; ++i){
    int iCode = stUTF8Decode((BYTE**)&str);
    if (0 == iCode){
      break;
    }
    if (iCode == -1){
      return -1;
    }
    else {
      uCodeArr[i] = iCode;
    }
  }
  uStep = i < uStep ? i : uStep;
  uint32_t l = i - uStep;
  uint32_t r = i;
  int bMatch = 0;
  int nMatch = 0;
  stDebug("test i=%u, l=%u, r=%u", i, l, r);
  for ( ; r > 0; ){
    uStep = r < uStep ? r : uStep;
    l = r - uStep;
    uint32_t j = l;
    bMatch = 0;
    for ( ; j < r; ++j){
      stDebug("test i=%u, l=%u, r=%u, j=%u", i, l, r, j);
      stDartsStateReset(handler, dState);
      uint32_t k = j;
      uint32_t oldMatch = nMatch;
      for ( ; k < r; ++k){
	stDebug("test i=%u, l=%u, r=%u, j=%u, k=%u, iCode=%u",
		i, l, r, j, k, uCodeArr[k]);
	ST_DARTS_STATE_SET_KEY(dState, uCodeArr[k]);
	int nFind = stDartsFindNext(handler, dState);
	if (nFind < 0){
	  stDebug("can't find icode=%u", uCodeArr[k]);
	  break;
	}
	else if (ST_DARTS_STATE_END(dState)){
	  stDebug("End");
	  break;
	}
	else if (ST_DARTS_STATE_HAS_VALUE(dState)){
	  // find word
	  stDebug("find word wordId=%d, l=%u, r=%u, j=%u",
		  ST_DARTS_STATE_VALUE(dState), l, r, j);
	  posArr[nMatch] = j;
	  wordIdArr[nMatch++] = ST_DARTS_STATE_VALUE(dState);
	}
	if ((r == k + 1) && ST_DARTS_STATE_HAS_VALUE(dState)){
	  bMatch = 1;
	  break;
	}
      }
      if (bMatch){
	r = j;
	break;
      }
      nMatch = oldMatch;
    }
    if (!bMatch){
	--r;
    }
  }
  *pLen = nMatch;
  return 0;
}


/** 
 * @brief automatic segmentation model
 * 
 * @param str the string length = uLen, the str length < 1024
 * @param wordIdArr wordId list, array length = 2 * uLen
 * @param posArr the word position, array length = 2 * uLen
 * @param uLen
 * @param bAsc 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsNextWord(st_darts* handler,
		st_darts_state* dState,
		struct st_wordInfo* pWordInfo)
{
  assert(NULL != handler && NULL != dState && NULL != pWordInfo);
  const char*  str = NULL;
  if (dState->uHasDecWords == 0){
    str = dState->start;
  }
  else {
    str = dState->cacheCode[(dState->uHasDecWords - 1)& MAX_ZH_WORD_MASK].pWordEnd;
  }
  const char* end = dState->end;
  if (str == NULL || end < str){
	stErr("str error. str=%p,end=%p", str, end);
	return -1;
  }
  while(1){
    // decode to cache
    for ( ; dState->uHasProcWords + MAX_ZH_WORD_LEN > dState->uHasDecWords 
		    && str < end ; ){
      int iCode = stUTF8Decode((BYTE**)&str);
      if (0 == iCode){
        break;
      }
      if (iCode == -1){
        return -1;
      }
      else {
        uint32_t uIdx = dState->uHasDecWords & MAX_ZH_WORD_MASK;
        dState->cacheCode[uIdx].uCode = iCode;
        dState->cacheCode[uIdx].pWordEnd = str;
		++dState->uHasDecWords;
      }
    }
    // match
    for ( ; dState->uCurWordPos < dState->uHasDecWords ; ++dState->uCurWordPos){
      uint32_t i = dState->uCurWordPos;
      stDebug("i=%u, hasProcWords=%u, hasDecWords=%u", 
		i, dState->uHasProcWords, dState->uHasDecWords);
	  struct term* pTerm = &dState->cacheCode[i & MAX_ZH_WORD_MASK];

	  ST_DARTS_STATE_SET_KEY(dState, pTerm->uCode);
	  int nFind = stDartsFindNext(handler, dState);
	  if (nFind < 0){
		  stDebug("can't find icode=%u", pTerm->uCode);
		  if (ST_DARTS_STATE_END(dState)){
			  stDebug("End");
		  }
		  break;
	  }
	  else if (ST_DARTS_STATE_HAS_VALUE(dState)){
		  // find word
		  pWordInfo->wordId = ST_DARTS_STATE_VALUE(dState);
		  pWordInfo->pWord = dState->start;
		  pWordInfo->wordLen = pTerm->pWordEnd - pWordInfo->pWord;
		  stDebug("find word=%s, wordId=%d, i=%u",
				  dState->start, ST_DARTS_STATE_VALUE(dState), i);
		  ++dState->uCurWordPos;
		  return 1;
	  }
	}
	dState->start = dState->cacheCode[dState->uHasProcWords & MAX_ZH_WORD_MASK].pWordEnd;
	++dState->uHasProcWords;
	dState->uCurWordPos = dState->uHasProcWords;
	stDartsStateReset(handler, dState);
	if (dState->uHasProcWords == dState->uHasDecWords ){
		break;
	}
  }

  return 0;
}


/** 
 * @brief automatic segmentation model
 * 
 * @param str the string length = uLen, the str length < 1024
 * @param wordIdArr wordId list, array length = 2 * uLen
 * @param posArr the word position, array length = 2 * uLen
 * @param uLen
 * @param bAsc 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsCutWordByte(st_darts* handler,
		st_darts_state* dState,
		const char* str,
		const char* end,
		struct st_wordInfo* pWordInfo,
		uint32_t* pWordCount,
		uint32_t uStep /* int bAsc */ )
{
  assert(NULL != str && 0 != *pWordCount && 0 != uStep);
  if (*pWordCount < ST_MAGIC_SENTENCE){
	stErr("WordInfo size too small. must >= %d", ST_MAGIC_SENTENCE);
	return -1;
  }
  struct term termCodeArr[ST_MAGIC_SENTENCE] = { {0, 0, 0} };
  int hasValue[ST_MAGIC_SENTENCE][16];
  int i = 0;
  const char* srcStr = str;
  const char* preStr = str;
  memset( hasValue, 0, sizeof(hasValue));
  *pWordCount= 0;
  for (i = 0; i < ST_MAGIC_SENTENCE ; ++i){
    if (str == end){
	break;
    }
    int iCode = stUTF8Decode((BYTE**)&str);
    if (0 == iCode){
      break;
    }
    if (iCode == -1){
      return -1;
    }
    else {
      termCodeArr[i].uCode = iCode;
      termCodeArr[i].pWord = preStr;
      termCodeArr[i].pWordEnd = str;
      preStr = str;
    }
  }
  uStep = i < uStep ? i : uStep;
  uint32_t l = i - uStep;
  uint32_t r = i;
  int nMatch = 0;
  stDebug("test i=%u, l=%u, r=%u", i, l, r);
  for ( ; r > 0; ){
    uStep = r < uStep ? r : uStep;
    l = r - uStep;
    uint32_t j = l;
    for ( ; j < r; ++j){
      stDebug("test i=%u, l=%u, r=%u, j=%u", i, l, r, j);
      stDartsStateReset(handler, dState);
      uint32_t k = j;
      for ( ; k < r; ++k){
	stDebug("test i=%u, l=%u, r=%u, j=%u, k=%u, iCode=%u",
		i, l, r, j, k, termCodeArr[k].uCode);
	ST_DARTS_STATE_SET_KEY(dState, termCodeArr[k].uCode);
	int nFind = stDartsFindNext(handler, dState);
	if (nFind < 0){
	  stDebug("can't find icode=%u", termCodeArr[k].uCode);
	  break;
	}
	else if (ST_DARTS_STATE_END(dState)){
	  stDebug("End");
	  break;
	}
	else if (ST_DARTS_STATE_HAS_VALUE(dState)){
	  // find word
	  stDebug("find word wordId=%d, l=%u, r=%u, j=%u",
		  ST_DARTS_STATE_VALUE(dState), l, r, j);
          unsigned int uLen = termCodeArr[k].pWordEnd - termCodeArr[j].pWord;
	  unsigned int uOff = termCodeArr[j].pWord - srcStr;
	  if (hasValue[uOff][uLen] == 0){
	    stDebug("hasValue map : uOff=%d, uLen=%d, havValue=%d", 
		   uOff, uLen, hasValue[uOff][uLen]);
	    hasValue[uOff][uLen] = 1;
	    pWordInfo[nMatch].wordId = ST_DARTS_STATE_VALUE(dState);
	    pWordInfo[nMatch].wordLen = uLen;
	    pWordInfo[nMatch++].pWord = termCodeArr[j].pWord;
	  }
	}
      }
    }
    --r;
  }
  *pWordCount = nMatch;
  return 0;
}
