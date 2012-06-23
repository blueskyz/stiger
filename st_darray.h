/**
 * @file   st_darray.h
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:26:07 2012
 * 
 * @brief  dynamic array interface
 * 
 * 
 */

#ifndef _ST_DARRAY_H_
#define _ST_DARRAY_H_

typedef struct st_darray st_darray;

/** 
 * @brief create darray object
 *
 * @param arraySize array size
 * @param objSize object size 
 * 
 * @return not null: pointer for darray, null: fail
 */
st_darray* stDArrayNew(uint32_t arraySize, uint32_t objSize);

/** 
 * @brief destroy darray
 * 
 * @param handler 
 * 
 * @return 0: succ, -1: fail
 */
int stDArrayFree(st_darray* handler);

/** 
 * @brief resize array size
 * 
 * @param handler 
 * @param arraySize 
 * 
 * @return 0: succ, -1: fail
 */
st_darray* stDArrayResize(st_darray* handler, uint32_t arraySize);

/** 
 * @brief put element to array
 * 
 * @param handler 
 * @param index 
 * @param elem 
 * 
 * @return elem
 */
void* stDArrayPut(st_darray* handler, unsigned int index, void* elem);

/** 
 * @brief get darray length
 * 
 * @param handler 
 * 
 * @return length
 */
uint32_t stDArrayLen(st_darray* handler);
uint32_t stDArrayUnitSize(st_darray* handler);
uint32_t stDArrayGetMemSize(st_darray* handler);

/** 
 * @brief get pointer for value
 * 
 * @param handler darray handler
 * @param index id
 * 
 * @return null: fail, not null: pointer for value
 */
void* stDArrayGet(st_darray* handler, unsigned int index);

void* stDArrayPointer(st_darray* handler);

#endif /* _ST_DARRAY_H_ */
