/**
 * @file   st_darray.c
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:52:17 2012
 * 
 * @brief  dynamic array implement
 * 
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "st_utils.h"
#include "st_darray.h"


struct st_darray
{
  uint32_t length;
  uint32_t unitSize;
  char array[0];
};

/** 
 * @brief create darray object
 *
 * @param arraySize array size
 * @param objSize object size 
 * 
 * @return not null: pointer for darray, null: fail
 */
st_darray* stDArrayNew(uint32_t arraySize, uint32_t objSize)
{
  if (arraySize <= 0 || objSize <=0){
    assert(arraySize <= 0 || objSize <=0);
    return NULL;
  }

  // note sizeof(st_darray) memory size ???
  int nSize = sizeof(st_darray) + (arraySize * objSize);
  st_darray* pdarray = (st_darray*)malloc(nSize);
  if (NULL == pdarray){
    assert(pdarray); 
    return NULL;
  }
  memset(pdarray, 0, nSize);
  pdarray->length = arraySize;
  pdarray->unitSize = objSize;
  
  return pdarray;
}

/** 
 * @brief destroy darray
 * 
 * @param handler 
 * 
 * @return 0: succ, -1:fail
 */
int stDArrayFree(st_darray* handler)
{
  if (NULL != handler){
    free(handler);
  }
  return 0;
}

st_darray* stDArrayResize(st_darray* handler, uint32_t arraySize)
{
  if (handler->length > arraySize){
    return NULL;
  }
  st_darray* pNewArr = stDArrayNew(arraySize, handler->unitSize);
  if (NULL == pNewArr){
    return NULL;
  }
  memcpy(pNewArr->array, handler->array, handler->length * handler->unitSize);
  return pNewArr;
}

/** 
 * @brief put element to array
 * 
 * @param handler 
 * @param index 
 * @param elem 
 * 
 * @return elem
 */
inline void* stDArrayPut(st_darray* handler, unsigned int index, void* elem)
{
  assert(handler && handler->array);
  assert(index >= 0 && index < handler->length);
  assert(elem);
  memcpy(handler->array + index * handler->unitSize, elem, handler->unitSize);
  return elem;
}

/** 
 * @brief get darray length
 * 
 * @param handler 
 * 
 * @return length
 */
inline uint32_t stDArrayLen(st_darray* handler)
{
  assert(handler && handler->array);
  return handler->length;
}

inline uint32_t stDArrayUnitSize(st_darray* handler)
{
  assert(handler && handler->array);
  return handler->unitSize;
}

inline uint32_t stDArrayGetMemSize(st_darray* handler)
{
  assert(handler && handler->array);
  if (NULL == handler){
    return 0;
  }
  return sizeof(st_darray) + handler->unitSize * handler->length;
}

/** 
 * @brief get pointer for value
 * 
 * @param handler darray handler
 * @param index id
 * 
 * @return pointer for value
 * @note index must >=0 and < handler->length
 */
inline void* stDArrayGet(st_darray* handler, unsigned int index)
{
  assert(handler && handler->array);
  assert(index >= 0 && index < handler->length);
  return handler->array + index * handler->unitSize;
}

inline void* stDArrayPointer(st_darray* handler)
{
  assert(handler && handler->array);
  return handler ? handler->array : NULL;
}
