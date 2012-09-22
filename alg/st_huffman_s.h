/*
 * static huffman interface
 */

#ifndef _HUFFMAN_S_H_
#define _HUFFMAN_S_H_

#include <stdint.h>

#include <alg/st_utils.h>

typedef struct st_hfms st_hfms;

st_hfms* stHfmSNew();

/*
 * @brief encode byte
 */
int stHfmSBuild(st_hfms* handler, BYTE* input, uint32_t uLen);

/*
 * @brief output zip content
 *
 * @param handler: the pointer for st_hfms
 * @param output: output buffer
 * @param uLen: the output buffer is length
 *
 * @return -1: error, 0: finish, >0: output size
 */
int stHfmSOutput(st_hfms* handler, BYTE* output, uint32_t* pLen);

/*
 * @brief free resource
 */
int stHfmSFree(st_hfms* handler);

/*
 * @brief memory... statistic information
 */
int stHfmsStatistic(st_hfms* handler);

int stHfmSDebug(st_hfms* handler);

#endif // _HUFFMAN_S_H_
