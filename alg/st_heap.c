/**
 * @file	st_heap.c
 * @author	Clark <zhangshizhuo@gmail.com>
 * @date	Thu Sep 13 00:30:20 PDT 2012
 * 
 * @brief	heap implement
 * 
 */

#include <alg/st_utils.h>
#include <alg/st_darray.h>
#include <alg/st_heap.h>

struct st_heap{
  st_darray* darray;
  uint32_t uMaxLen;
  uint32_t uUseLen;
  stHeapCmp_t cmp;
};

st_heap* stHeapNew(uint32_t uMaxLen, stHeapCmp_t cmp)
{
  st_heap* handler = (st_heap*)malloc(sizeof(st_heap));
  if (NULL == handler){
	stErr("stHeapNew malloc fail.");
	return NULL;
  }
  memset(handler, 0, sizeof(st_heap));
  handler->darray = stDArrayNew(uMaxLen, sizeof(void*));
  if (NULL == handler->darray){
	stErr("stHeapNew new darray fail.");
	free(handler);
	return NULL;
  }
  handler->uMaxLen = uMaxLen;
  handler->uUseLen = 0;
  handler->cmp = cmp;
  return handler;
}

int stHeapPush(st_heap* handler, void* value)
{
  if (handler->uUseLen >= handler->uMaxLen){
	stErr("stHeapPush full error, use=%u, maxlen=%u", handler->uUseLen, handler->uMaxLen);
	return -1;
  }
  stDArrayPut(handler->darray, handler->uUseLen, (void*)&value);
  uint32_t curPos = handler->uUseLen;
  uint32_t parent = (0 == curPos) ? 0 : (curPos - 1) / 2;
  while(0 != curPos){
	parent = (curPos - 1) >> 1;
	void** parentValue = (void**)stDArrayGet(handler->darray, parent);
	void** curValue = (void**)stDArrayGet(handler->darray, curPos);
	if (handler->cmp(*parentValue, *curValue) >= 0){
	  break;
	}

	// exchange
	void* tmp = *parentValue;
	*parentValue = *curValue;
	*curValue = tmp;

	curPos = parent;
  }
#ifdef DEBUG_DETAIL
  stDebug("parent=%u, curPos=%u", parent, curPos);
#endif
  handler->uUseLen++;

  return 0;
}

void* stHeapPop(st_heap* handler)
{
  if (NULL == handler){
	stErr("stHeapPop handler is null.");
	return NULL;
  }
  if (0 == handler->uUseLen){
	return NULL;
  }
  void* value = *(void**)stDArrayGet(handler->darray, 0);
  handler->uUseLen--;

  if (handler->uUseLen >= 1){
	void** curValue = (void**)stDArrayGet(handler->darray, handler->uUseLen);
	stDArrayPut(handler->darray, 0, curValue);

	// modify heap
	uint32_t i = 0;
	uint32_t uLast = handler->uUseLen - 1;
	while((i << 1) + 1 <= uLast){
	  uint32_t curPos = (i << 1) + 1;
	  void** curVal = (void**)stDArrayGet(handler->darray, i);

	  void** nextVal = (void**)stDArrayGet(handler->darray, curPos);
	  if (curPos < uLast){
		void** rightVal = (void**)stDArrayGet(handler->darray, curPos + 1);
		if (handler->cmp(*nextVal, *rightVal) < 0){
		  ++curPos;
		  nextVal = rightVal;
		}
	  }
	  if (handler->cmp(*curVal, *nextVal) >= 0){
		break;
	  }

	  // exchange
	  void* tmp = *nextVal;
	  *nextVal = *curVal;
	  *curVal = tmp;
	  i = curPos;
	}
  }

  return value;
}

int stHeapBuild(st_heap* handler)
{
  return 0;
}

int stHeapFree(st_heap* handler)
{
  if (NULL == handler){
	stErr("free null heap.");
	return -1;
  }
  if (NULL == handler->darray){
	stErr("free null darray with heap.");
  }
  else {
	stDArrayFree(handler->darray);
  }
  free(handler);
  return 0;
}

