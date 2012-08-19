/**
 * @file   st_darts.h
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:26:07 2012
 * 
 * @brief  double array trie interface
 * 
 * 
 */

#ifndef _ST_DARTS_H_
#define _ST_DARTS_H_

#include <stdint.h>

typedef struct st_darts st_darts;
typedef struct st_darts_state st_darts_state;

typedef enum { eds_begin = 0x01,
	       eds_mid = 0x02,
	       eds_end = 0x04,
	       eds_has_value = 0x08 } st_e_darts_state;

struct st_darts_state
{
  unsigned int uMagic;
  uint16_t uKey;
  unsigned int uValue;
  unsigned int uSState;
  int state;
};

#define ST_DARTS_STATE_CMP(st_state, state_value) \
  ((st_state->state & state_value) == state_value)

#define ST_DARTS_STATE(st_state) st_state->state
#define ST_DARTS_STATE_SET_KEY(st_state, key) (st_state->uKey = key)
#define ST_DARTS_STATE_BEGIN(st_state) ST_DARTS_STATE_CMP(st_state, eds_begin)
#define ST_DARTS_STATE_MID(st_state) ST_DARTS_STATE_CMP(st_state, eds_mid)
#define ST_DARTS_STATE_END(st_state) ST_DARTS_STATE_CMP(st_state, eds_end)

#define ST_DARTS_STATE_HAS_VALUE(st_state)				\
  ST_DARTS_STATE_MID(st_state)						\
  && ST_DARTS_STATE_CMP(st_state, eds_has_value)
#define ST_DARTS_STATE_VALUE(st_state) st_state->uValue

/** 
 * @brief create double array trie
 * 
 * 
 * @return not null: succ, null: fail
 */
st_darts* stDartsNew(unsigned int baseArrayLen);

/** 
 * @brief destroy double array trie
 * 
 * @param handler the pointer for st_darts
 * 
 * @return 0: succ, -1: fail
 */
int stDartsFree(st_darts* handler);

/** 
 * @brief save darts to file
 * 
 * @param handler pointer for st_darts
 * @param filePath the filePath
 * @see stDartsLoad()
 * 
 * @return 0: succ, -1: fail
 */
int stDartsSave(st_darts* handler, const char* filePath);

/** 
 * @brief load darts from file
 * 
 * @param filePath
 * @see stDartsSave()
 * 
 * @return not null: succ, NULL: fail
 */
st_darts* stDartsLoad(const char* filePath);

/** 
 * @brief create search state
 * 
 * @param handler 
 * 
 * @return not null: succ/the pointer for state, null: fail
 */
st_darts_state* stDartsStateNew(st_darts* handler);

/** 
 * @brief destroy state object
 * 
 * @param handler 
 * @param state 
 * 
 * @return 
 */
int stDartsStateFree(st_darts_state* handler, st_darts_state* state);

/** 
 * @brief reset state for reuse
 *
 * @param handler
 * @param state 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsStateReset(st_darts* handler, st_darts_state* state);

/** 
 * @brief find value, state for check
 * 
 * @param handler 
 * @param state 
 * 
 * @return 0: succ, -1: fail
 */
int stDartsFindNext(st_darts* handler, st_darts_state* state);

void stDartsStatistics(st_darts* handler, uint32_t uStep);

/** 
 * @brief add key and value to darts
 * 
 * @param handler 
 * @param uKey the key for search
 * @param uValue the value for other infomation( eg: pointer for ... )
 * 
 * @return 0: succ, -1: fail
 */
int stDartsPut(st_darts* handler,
	       uint16_t* pKey,
	       unsigned int uLen,
	       unsigned int uValue);


int stCutWord(st_darts* handler,
	      st_darts_state* dState,
	      const char* str,
	      uint32_t* wordIdArr,
	      uint32_t* posArr,
	      uint32_t* pLen,
	      uint32_t uStep /* int bAsc */ );

int stCutWordByte(st_darts* handler,
		st_darts_state* dState,
		const char* str,
		const char** wordArr,
		uint32_t* wordLenArr,
		uint32_t* pLen,
		uint32_t uStep /* int bAsc */ );

#endif /* _ST_DARTS_H_ */
