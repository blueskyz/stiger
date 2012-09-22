/**
 * @file	st_heap.h
 * @author	Clark <zhangshizhuo@gmail.com>
 * @date	Wed Sep 12 19:05:17 PDT 2012
 * 
 * @brief	heap interface
 * 
 */

#ifndef _ST_HEAP_H_
#define _ST_HEAP_H_

#include <alg/st_utils.h>

typedef struct st_heap st_heap;

typedef int (*stHeapCmp_t)(void* v1, void* v2);

st_heap* stHeapNew(uint32_t uLen, stHeapCmp_t cmp);

int stHeapPush(st_heap* handler, void* value);

void* stHeapPop(st_heap* handler);

int stHeapBuild(st_heap* handler);

int stHeapFree(st_heap* handler);

#endif  // _ST_HEAP_H_
