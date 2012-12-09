/**
 * @file   st_darray.h
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:26:07 2012
 * 
 * @brief  static huffman
 * 
 * 
 */

#include <math.h>
#include <alg/st_huffman_s.h>
#include <alg/st_heap.h>

typedef enum { ST_HFMS_TREE_NULL, ST_HFMS_TREE_NODE, ST_HFMS_TREE_LEAF } st_hfms_tree_type;

#define ST_HUFFMAN_MAGIC_VER 0x0001
// data repeat 256: 3~6 tag, 257: 3~10 tag, 258: 11~256 259: end tag
// code repeat 28: 3~6 tag, 29: 3~10 tag, 30: 11~256 
#define DATA_SIZE (0x01 << 8)
#define CODE_SIZE DATA_SIZE + 4
#define ST_HUFFMAN_MAX_LEVEL 0x10

typedef struct st_hfms_node st_hfms_node;
struct st_hfms_node{
  st_hfms_tree_type _type;
  st_hfms_node* _parent;
  uint32_t _count; // count of all child
  uint32_t _uBitCount; // level == bit length
  st_hfms_node* _child[2]; // 0: left, 1: right
};

typedef struct st_hfms_leaf {
  st_hfms_tree_type _type;
  st_hfms_node* _parent;
  uint32_t _count; // weight 
  uint32_t _uBitCount; // level == bit length
  uint32_t _code;
} st_hfms_leaf;

typedef struct st_hfms_result {
	uint32_t _uHeaderSize;
	uint32_t _uCompressSize;
	double _dCompressRate;
	double _dBestRate;
} st_hfms_result;

struct st_hfms{
  st_hfms_node* _tree;
  uint16_t _uHeight;
  uint32_t _uLen;
  BYTE* _data;
  st_hfms_result _status;
  st_hfms_leaf _leaf[CODE_SIZE];
  st_hfms_node _node[CODE_SIZE];
  uint32_t _arrLevelLeaf[ST_HUFFMAN_MAX_LEVEL+1];
};

#define ST_HFMS_IS_NODE(pNode) (((st_hfms_node*)pNode)->_type == ST_HFMS_TREE_NODE)
#define ST_HFMS_IS_LEAF(pLeaf) (((st_hfms_leaf*)pLeaf)->_type == ST_HFMS_TREE_LEAF)
#define ST_HFMS_CHAR_VAL(handler, pLeaf) (uint32_t)((st_hfms_leaf*)pLeaf - handler->_leaf)


//------------------------------------------------------
// private
static int stHfmsByteUnitCmp(void* one, void* two)
{
  st_hfms_node* p1 = (st_hfms_node*)one;
  st_hfms_node* p2 = (st_hfms_node*)two;
  if (p1->_count < p2->_count){
      return 1;
  }
  return -1;
}

static int stHfmSCreateTree(st_hfms* handler)
{
  // statistic
  uint32_t maxCount = 0;
  BYTE* input = handler->_data;
  for (uint32_t i = 0 ; i < handler->_uLen ; ++i){
	  if (input[i] > 128){
		  stDebug("warning %u ascii=%u, %c", i, input[i], input[i]);
	  }
	st_hfms_leaf* leaf = &handler->_leaf[input[i]];
	if (leaf->_count == 0){
	  leaf->_type = ST_HFMS_TREE_LEAF;
	  leaf->_parent = NULL;
	  leaf->_uBitCount = 0;
	}
	++leaf->_count;
	maxCount = max(leaf->_count, maxCount);
  }
  stDebug("maxCount %u", maxCount);

  // add eof tag
  handler->_leaf[CODE_SIZE-4]._type = ST_HFMS_TREE_LEAF;
  handler->_leaf[CODE_SIZE-4]._parent = NULL;
  handler->_leaf[CODE_SIZE-4]._uBitCount = 0;
  handler->_leaf[CODE_SIZE-4]._count = 1;
  handler->_leaf[CODE_SIZE-3]._type = ST_HFMS_TREE_LEAF;
  handler->_leaf[CODE_SIZE-3]._parent = NULL;
  handler->_leaf[CODE_SIZE-3]._uBitCount = 0;
  handler->_leaf[CODE_SIZE-3]._count = 1;
  handler->_leaf[CODE_SIZE-2]._type = ST_HFMS_TREE_LEAF;
  handler->_leaf[CODE_SIZE-2]._parent = NULL;
  handler->_leaf[CODE_SIZE-2]._uBitCount = 0;
  handler->_leaf[CODE_SIZE-2]._count = 1;
  // eof
  handler->_leaf[CODE_SIZE-1]._type = ST_HFMS_TREE_LEAF;
  handler->_leaf[CODE_SIZE-1]._parent = NULL;
  handler->_leaf[CODE_SIZE-1]._uBitCount = 0;
  handler->_leaf[CODE_SIZE-1]._count = 1;

  // statistic shang
  double shang = 0.0f;
  uint32_t scale = 0;
  for (int i = 0 ; i < DATA_SIZE ; ++i){
	  if (0 == handler->_leaf[i]._count){
		  continue;
	  }
	  double dShang = - log(1.0 * handler->_leaf[i]._count / handler->_uLen) / log(2);
	  shang += dShang * handler->_leaf[i]._count;

	  scale = (int)(handler->_leaf[i]._count / ((double)maxCount / (double)256));
	  if (0 == scale){
		  handler->_leaf[i]._count = 1;
	  }
	  else {
		  handler->_leaf[i]._count = scale;
	  } 
  }
  printf("shang ---- %f, %f, %f\n", log(2), log(0.4), shang);
  handler->_status._dBestRate = (1 - (shang / (handler->_uLen << 3))) * 100;

  st_heap* heapHandler = stHeapNew(CODE_SIZE, stHfmsByteUnitCmp);
  if (NULL == heapHandler){
	stErr("new heap fail.");
	goto fail;
  }
  for (uint32_t i = 0 ; i < CODE_SIZE ; ++i){
	/*
	stDebug("i=%u, push ascii=%u, count=%u", 
		i, handler->unit[i].byte, handler->unit[i].count);
		*/
	if (0 != handler->_leaf[i]._count){
	    stHeapPush(heapHandler, &handler->_leaf[i]);
	}
  }

#ifdef DEBUG
  // output content
  stDebug("output byte content");
  for (uint32_t i = 0 ; i < CODE_SIZE ; ++i){
	if (0 == handler->_leaf[i]._count) continue;
	if (i < 128){
	  stDebug("ascii=%u, char=%c, times=%u", i, i, handler->_leaf[i]._count);
	}
	else {
	  stDebug("ascii=%u, times=%u", i, handler->_leaf[i]._count);
	}
  }
#endif
  
  // build huffman tree
  stDebug("create huffman tree");
  uint32_t u2 = 0;
  uint32_t uNodeIdx = 0;
  st_hfms_leaf* pUnit[2] = { NULL };
  pUnit[u2] = stHeapPop(heapHandler);
  while(NULL != pUnit[u2]){
      /*
      // debug log
      if (ST_HFMS_IS_LEAF(pUnit[u2])){
      stDebug("0 ascii=%u, count=%u", 
      ST_HFMS_CHAR_VAL(handler, pUnit[u2]),
      pUnit[u2]->count);
      }
      */
      // debug end

	  if(0 == u2){
		  pUnit[++u2] = stHeapPop(heapHandler);
	  }
	  if ((NULL == pUnit[1]) && ST_HFMS_IS_NODE(pUnit[0])){ // u2 == 1
		  // end
		  stDebug("find tree root");
		  handler->_tree = (st_hfms_node*)pUnit[0];
		  handler->_tree->_uBitCount = 0;
		  handler->_uHeight = 0;
		  break;
	  }
	  // process and push back
	  if (NULL != pUnit[0]){
		  st_hfms_node* pNode = &handler->_node[uNodeIdx];
		  pNode->_type = ST_HFMS_TREE_NODE;
		  pNode->_parent = NULL;
		  pUnit[0]->_parent = pNode;
		  pNode->_child[0] = (st_hfms_node*)pUnit[0];
		  pNode->_count = pUnit[0]->_count;
		  if (NULL != pUnit[1]){
			  pUnit[1]->_parent = pNode;
			  pNode->_count += pUnit[1]->_count;
			  pNode->_child[1] = (st_hfms_node*)pUnit[1];
		  }
		  stHeapPush(heapHandler, pNode);

		  // debug log
#ifdef DEBUG
		  for (int i = 0 ; i < 2 ; ++i){
			  if (NULL != pUnit[i] && ST_HFMS_IS_LEAF(pUnit[i])){
				  stDebug("0 ascii=%u, count=%u", 
						  ST_HFMS_CHAR_VAL(handler, pUnit[i]),
						  pUnit[i]->_count);
			  }
		  }
		  stDebug("create node uNodeIdx=%u, count=%u", uNodeIdx, pNode->_count);
#endif
		  // debug end
		  ++uNodeIdx;
	  }

	  // next
	  u2 = 0;
	  pUnit[u2] = stHeapPop(heapHandler);
  }

  // free resource
  stHeapFree(heapHandler);
  return 0;

fail:
  return -1;
}

static int stHfmSCalcBitCode(st_hfms* handler)
{
	assert(NULL != handler);
	st_hfms_leaf* leaf = (st_hfms_leaf*)handler->_leaf;
	// reset level leaf count
	memset(handler->_arrLevelLeaf, 0, sizeof(handler->_arrLevelLeaf));
	for (int i = 0 ; i < CODE_SIZE ; ++i){
		if (0 != leaf[i]._uBitCount){
			++handler->_arrLevelLeaf[leaf[i]._uBitCount];
		}
	}

	// create encode
	uint32_t uLastCode = 0;
	uint32_t uLastLevel = 0;
	uint32_t uFirst = 1;
	for (int iLvl = 0 ; iLvl <= ST_HUFFMAN_MAX_LEVEL ; ++iLvl){
		if (0 == handler->_arrLevelLeaf[iLvl]){
			continue;
		}
		uint32_t uLvlCount = handler->_arrLevelLeaf[iLvl];
		stDebug("tree have leafs in level %d, counts=%u", iLvl, uLvlCount);
		for (int i = 0 ; i < CODE_SIZE && 0 != uLvlCount ; ++i){
			if (0 == leaf[i]._uBitCount || iLvl != leaf[i]._uBitCount){
				continue;
			}
			if ((1 == uFirst) && (0 == uLastCode)){
				// first leaf
				uLastCode = 0; // notice _uBitCount
				uLastLevel = iLvl;
				uFirst = 0;
			}
			else if (uLastLevel != iLvl){
				uLastCode = (uLastCode + 1) << (iLvl - uLastLevel);
				uLastLevel = iLvl;
			}
			else {
				uLastCode += 1;
			}
			leaf[i]._code = uLastCode;
			stDebug("tree ascii=%d code=%u level=%d bitcount=%d curCounts=%u", 
					ST_HFMS_CHAR_VAL(handler, &leaf[i]), 
					leaf[i]._code,
					iLvl,
					leaf[i]._uBitCount,
					uLvlCount);
			--uLvlCount;
		}
		uLastLevel = iLvl;
	}

#ifdef DEBUG
	for (int iLvl = 0 ; iLvl <= ST_HUFFMAN_MAX_LEVEL ; ++iLvl){
		if (0 == handler->_arrLevelLeaf[iLvl]){
			continue;
		}
		char sEncode[64] = { 0 };
		for (int i = 0 ; i < CODE_SIZE ; ++i){
			if (iLvl != leaf[i]._uBitCount){
				continue;
			}
			uint32_t uCode = leaf[i]._code;
			for (int j = 0 ; j < leaf[i]._uBitCount ; ++j){
				if (0 == (uCode & (0x1 << (leaf[i]._uBitCount - j - 1)))){
					sEncode[j] = '0';
				}
				else {
					sEncode[j] = '1';
				}
			}
			sEncode[leaf[i]._uBitCount] = '\0';
			stDebug("ascii=%d, savecode=%x, level=%d, encode=%s, count=%d", 
					ST_HFMS_CHAR_VAL(handler, &leaf[i]), 
					leaf[i]._code, leaf[i]._uBitCount, sEncode, leaf[i]._count);
		}
	}
#endif

	return 0;
}

int stHfmSCalcLevel(st_hfms* handler)
{
	static const char* sDir[] = {"left", "right"};
	uint32_t uDir = 0;
	const uint32_t uLeft = 0;
	const uint32_t uRight = 1;
	st_hfms_node* node = handler->_tree;
	stDebug("tree node root=%p", node);
	while(NULL != node){
		if (ST_HFMS_IS_NODE(node)){
			if (NULL != node->_child[uLeft] 
					&& 0 == node->_child[uLeft]->_uBitCount){
				uDir = uLeft;
			}
			else if (NULL != node->_child[uRight] 
					&& 0 == node->_child[uRight]->_uBitCount){
				uDir = uRight;
			}
			else {
				node = node->_parent;
				continue;
			}
			stDebug("tree %s node level=%d", sDir[uDir], node->_uBitCount);
			node = node->_child[uDir];
			//node->uBitCount = node->parent->uBitCount << 1 | uDir;
			node->_uBitCount = node->_parent->_uBitCount + 1;
		}
		else if (ST_HFMS_IS_LEAF(node)){
			stDebug("tree %s level=%d leaf ascii=%d", sDir[uDir], node->_uBitCount,
					ST_HFMS_CHAR_VAL(handler, node));
			handler->_uHeight = max(node->_uBitCount, handler->_uHeight);
			++handler->_arrLevelLeaf[node->_uBitCount];
			node = node->_parent;
		}
		else {
			assert(0);
		}
	}
	stDebug("tree height=%d", handler->_uHeight);

	return 0;
}

static int stHfmSSortTree(st_hfms* handler)
{
	assert(handler);
	if (handler->_uHeight <= ST_HUFFMAN_MAX_LEVEL){
		stDebug("have not leaf out of maxLevel=%d, tree height=%d",
				ST_HUFFMAN_MAX_LEVEL, handler->_uHeight);
		return 0;
	}

	// fixme, haha
	stDebug("already abort()");
	abort();

	// all leaf's level <= ST_HUFFMAN_MAX_LEVEL
	st_hfms_leaf* leaf = handler->_leaf;
	BYTE uCurLvlOff = 0;
	stDebug("tree move and sort");
	for (int i = 0 ; i < CODE_SIZE ; ++i){
		if (leaf[i]._uBitCount <= ST_HUFFMAN_MAX_LEVEL){
			continue;
		}
		uint32_t uLvl = ST_HUFFMAN_MAX_LEVEL - 1;
		while(1){
			if (0 == leaf[uCurLvlOff]._uBitCount){
				if (++uCurLvlOff >= CODE_SIZE){
					uCurLvlOff = 0;
				}
				continue;
			}
			if (0 == handler->_arrLevelLeaf[uLvl]) {
				if (0 == uLvl){
					// error
					stErr("can't find leaf for move");
					return -1;
				}
				else {
					--uLvl;
				}
				continue;
			}

			if (leaf[uCurLvlOff]._uBitCount == uLvl){
				// find leaf for swap
				--handler->_arrLevelLeaf[uLvl];
				handler->_arrLevelLeaf[uLvl+1] += 2;
				stDebug("tree Level=%d leafs=%d uCurLvlOff=%d", 
						uLvl, handler->_arrLevelLeaf[uLvl], uCurLvlOff);
				break;
			}
			if (++uCurLvlOff > CODE_SIZE){
				uCurLvlOff = 0;
			}
		}
		// ----------------------------------------------------
		// move leaf
		st_hfms_node* parentNodeFrom = leaf[i]._parent;
		// get src sibling
		st_hfms_node* siblingNodeFrom = parentNodeFrom->_child[0];
		if (siblingNodeFrom == (st_hfms_node*)&leaf[i]){
			siblingNodeFrom = parentNodeFrom->_child[1];
		}
		// remove leaf and move sibling
		st_hfms_node* grandfatherNodeFrom = parentNodeFrom->_parent;
		if (grandfatherNodeFrom->_child[0] == parentNodeFrom){
			grandfatherNodeFrom->_child[0] = siblingNodeFrom;
		}
		else {
			grandfatherNodeFrom->_child[1] = siblingNodeFrom;
		}
		grandfatherNodeFrom->_count -= leaf[i]._count;
		siblingNodeFrom->_parent = grandfatherNodeFrom;
		siblingNodeFrom->_uBitCount = grandfatherNodeFrom->_uBitCount + 1;
		parentNodeFrom->_count -= leaf[i]._count;

		// ----------------------------------------------------
		// add leaf
		st_hfms_node* parentNodeTo = leaf[uCurLvlOff]._parent;
		if (parentNodeTo->_child[0] == (st_hfms_node*)&leaf[uCurLvlOff]){
			parentNodeTo->_child[0] = parentNodeFrom;
		}
		else {
			parentNodeTo->_child[1] = parentNodeFrom;
		}
		parentNodeTo->_count += leaf[uCurLvlOff]._count;
		parentNodeFrom->_parent = parentNodeTo;
		parentNodeFrom->_uBitCount = parentNodeTo->_uBitCount + 1;
		if (parentNodeFrom->_child[0] == (st_hfms_node*)&leaf[i]){
			parentNodeFrom->_child[1] = (st_hfms_node*)&leaf[uCurLvlOff];
		}
		else {
			parentNodeFrom->_child[0] = (st_hfms_node*)&leaf[uCurLvlOff];
		}
		parentNodeFrom->_count += leaf[uCurLvlOff]._count;
		leaf[uCurLvlOff]._parent = parentNodeFrom;
		leaf[uCurLvlOff]._uBitCount = parentNodeFrom->_uBitCount + 1;
		leaf[i]._uBitCount = parentNodeFrom->_uBitCount + 1;
		// ----------------------------------------------------
	}

	// sort small --> big for leaf in level
	return 0;
}

// buf enough
static int stSetBitVal(BYTE** ppBuf, uint32_t* pBitOffset, 
		uint32_t uValue, uint32_t uBitLen)
{
	assert(*ppBuf && (*pBitOffset < 8) && (0 != uBitLen));

	BYTE* buf = *ppBuf;
	uint32_t uIdx = 0;
	uint32_t uLeftBit = 8 - (*pBitOffset & 0x07);
	while (uBitLen > 0){
		// process
		buf[uIdx] &= (0xff << uLeftBit);
		if (uBitLen >= uLeftBit){
			uBitLen -= uLeftBit;
			buf[uIdx] |= (uValue >> uBitLen);
			uValue &= (~0x0 >> (sizeof(uint32_t) - uBitLen));
			++uIdx;
			uLeftBit = 8;
		}
		else {
			uLeftBit -= uBitLen;
			uBitLen = 0;
			buf[uIdx] |= (uValue << uLeftBit);
		}
	}

	// reset length status
	*ppBuf += uIdx;
	*pBitOffset = 8 - uLeftBit;
	return 0;
}

static int stHfmSCompress(st_hfms* handler, BYTE* out, uint32_t* pOutLen)
{
	assert(NULL != handler && NULL != out && 0 != *pOutLen);
	if (NULL == out || 0 == pOutLen){
		return -1;
	}

	uint32_t idx = 0;
	// write version
	out[idx++] = ST_HUFFMAN_MAGIC_VER;
	// fixme opt: 1. static, dynamic 2.fast speed, small size 3. 1/4 table compress alg ...
	out[idx++] = 0x00;

	// save encode table ( only save encode length)
	// code repeat 28(0x1c): 3~6 tag, 2 bit times
	// 29(0x1d): 3~10 tag, 3 bit times
	// 30(0x1e): 11~256, 8 bit times
	st_hfms_leaf* leaf = handler->_leaf;
	BYTE lastBitLen = leaf[0]._uBitCount;
	uint32_t uLastPos = 0;
	uint32_t uSetCounts = 1;
	uint32_t uBitPos = 0;
	BYTE* outBytePos = &out[idx];
	for ( int i = 1 ; i <= CODE_SIZE ; ++i ){
		BYTE* curOutBytePos = outBytePos;
		uint32_t uCurBitPos = uBitPos;
		if ((lastBitLen == leaf[i]._uBitCount) 
				&& (uSetCounts <= 256)
				&& (i < CODE_SIZE)){
			++uSetCounts;
		}
		else { // encode to table
			if (uSetCounts >= 3){ // have repeat
				// write encode
				stSetBitVal(&outBytePos, &uBitPos, lastBitLen, 5);
				// write repeat tag: 00(+2bit repeats) => {3,6}, 
				// 10(+3bit repeats) => {3, 10}
				// 11(+8bit repeats) => {3, 256}
				if (uSetCounts >= 3 && uSetCounts <= 6){
					stSetBitVal(&outBytePos, &uBitPos, 0x1c, 5);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 2);
				}
				else if (uSetCounts >= 3 && uSetCounts <= 10){
					stSetBitVal(&outBytePos, &uBitPos, 0x1d, 5);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 3);
				}
				else {
					stSetBitVal(&outBytePos, &uBitPos, 0x1e, 5);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 8);
				}
				stDebug("write off=%u, bitlength=%u, repeats=%u, size=%u bit",
						uLastPos, 
						lastBitLen, 
						uSetCounts,
						((outBytePos - curOutBytePos) << 3) + uBitPos - uCurBitPos);
			}
			else {
				for (int j = 0 ; j < uSetCounts ; ++j){
					// write encode
					stSetBitVal(&outBytePos, &uBitPos, lastBitLen, 5);
				}
				stDebug("write off=%u, bitlength=%u, times=%u, size=%u bit",
						uLastPos, 
						lastBitLen, 
						uSetCounts,
						((outBytePos - curOutBytePos) << 3) + uBitPos - uCurBitPos);

			}
			if (i < CODE_SIZE){
				stDebug("current lastBitLen=%u, next bitlength=%u",
						lastBitLen, leaf[i]._uBitCount);
				lastBitLen = leaf[i]._uBitCount;
				uSetCounts = 1;
				uLastPos = i;
			}
		}
	}
	handler->_status._uHeaderSize = (outBytePos - out) + 1;

	// compress data
	BYTE* curBytePos = outBytePos;
	BYTE* input = handler->_data;
	uint32_t uLastIdx = 0;
	BYTE data = input[uLastIdx];
	uLastPos = 0;
	uSetCounts = 1;
	/*
	for (uint32_t i = 1 ; i < handler->_uLen ; ++i){
		leaf = &handler->_leaf[input[i]];
		stSetBitVal(&outBytePos, &uBitPos, leaf->_code, leaf->_uBitCount);
	}
	*/
	for (uint32_t i = 1 ; i <= handler->_uLen ; ++i){
		if (input[uLastIdx] == input[i] && uSetCounts <= 256 && i < handler->_uLen){ 
			++uSetCounts;
		}
		else {
			data = input[uLastIdx];
			if ((uSetCounts >= 3) 
					&& ((uSetCounts-1) * leaf[data]._uBitCount > ST_HUFFMAN_MAX_LEVEL)){
				// have repeat
				// write data
				stSetBitVal(&outBytePos, &uBitPos, 
						leaf[data]._code, leaf[data]._uBitCount);
				if (uSetCounts >= 3 && uSetCounts <= 6){
					stSetBitVal(&outBytePos, &uBitPos, 
							leaf[256]._code, leaf[256]._uBitCount);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 2);
				}
				else if (uSetCounts >= 3 && uSetCounts <= 10){
					stSetBitVal(&outBytePos, &uBitPos, 
							leaf[257]._code, leaf[257]._uBitCount);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 3);
				}
				else {
					stSetBitVal(&outBytePos, &uBitPos, 
							leaf[258]._code, leaf[258]._uBitCount);
					stSetBitVal(&outBytePos, &uBitPos, uSetCounts - 3, 8);
				}
				stDebug("value=%c, uLastIdx=%u, repeats=%u, bit size=%u, " \
						"256=%u, 257=%u, 258=%u",
						data, uLastIdx, uSetCounts, leaf[data]._uBitCount,
						leaf[256]._uBitCount, leaf[257]._uBitCount, leaf[258]._uBitCount);
			}
			else {
				for (int j = 0 ; j < uSetCounts ; ++j){
					// write encode
					stSetBitVal(&outBytePos, &uBitPos, 
							leaf[data]._code, leaf[data]._uBitCount);
				}
				stDebug("no length value=%c, uLastIdx=%u, repeats=%u, bit size=%u, " \
						"256=%u, 257=%u, 258=%u",
						data, uLastIdx, uSetCounts, leaf[data]._uBitCount,
						leaf[256]._uBitCount, leaf[257]._uBitCount, leaf[258]._uBitCount);
			}
			if (i < handler->_uLen){
				// stDebug("current uLastIdx=%u, next Idx=%u", uLastIdx, i);
				uLastIdx = i;
				uSetCounts = 1;
				uLastPos = i;
			}
		}
	}

	// write eof
	leaf = &handler->_leaf[259];
	stSetBitVal(&outBytePos, &uBitPos, leaf->_code, leaf->_uBitCount);

	handler->_status._uCompressSize = (outBytePos - curBytePos) + 1;
	handler->_status._dCompressRate = 100 - 100.0 * handler->_status._uCompressSize 
		/ handler->_uLen;

	// compress result status
	stDebug("compress content size=%u bytes, header size=%u, " \
			"out size=%u, rate=%f, bestRate=%f", 
			handler->_uLen,
			handler->_status._uHeaderSize,
			handler->_status._uCompressSize,
			handler->_status._dCompressRate,
			handler->_status._dBestRate);

	return 0;
}

//------------------------------------------------------
// public interface
st_hfms* stHfmSNew()
{
	st_hfms* handler = (st_hfms*)malloc(sizeof(st_hfms));
	if (NULL == handler){
		stErr("stHfmSNew malloc fail.");
		return NULL;
	}
	memset(handler, 0, sizeof(st_hfms));
	handler->_tree = NULL;
	return handler;
}

int stHfmSBuild(st_hfms* handler, BYTE* input, uint32_t uLen, BYTE* out, uint32_t* pOutLen)
{
	// check argument
	if (NULL == input){
		stErr("input data is NULL.");
		goto fail;
	}
	if (0 == uLen){
		stErr("input data length is %u", uLen);
		goto fail;
	}
	handler->_data = input;
	handler->_uLen = uLen;

	// create huffman tree
	if (stHfmSCreateTree(handler) == -1){
		goto fail;
	}
	if (stHfmSCalcLevel(handler) == -1){
		goto fail;
	}
	// sort tree
	if (stHfmSSortTree(handler) == -1){
		goto fail;
	}
	// bit encode
	if (stHfmSCalcBitCode(handler) == -1){
		goto fail;
	}
	// compress
	stDebug("out: %p, outlen=%d", out, *pOutLen);
	if (NULL == out || 0 == *pOutLen){
		// output later --> call stHfmSOutput
	}
	else if (stHfmSCompress(handler, out, pOutLen) == -1){
		goto fail;
	}
	return 0;

fail:
	return -1;
}

int stHfmSOutput(st_hfms* handler, BYTE* output, uint32_t* pLen)
{
	return -1;
}

int stHfmSFree(st_hfms* handler)
{
	if (NULL != handler){
		free(handler);
	}
	return 0;
}

int stHfmSDebug(st_hfms* handler)
{
  static const char* sDir[] = {"left", "right"};
  uint32_t uDir = 0;
  const uint32_t uLeft = 0;
  const uint32_t uRight = 1;
  st_hfms_node* node = handler->_tree;
  stDebug("tree node root=%p", node);
  while(NULL != node){
      if (ST_HFMS_IS_NODE(node)){
	  if (0 == node->_child[uLeft]->_uBitCount){
	      uDir = uLeft;
	  }
	  else if (0 == node->_child[uRight]->_uBitCount){
	      uDir = uRight;
	  }
	  else {
	      node = node->_parent;
	      continue;
	  }
	  stDebug("tree %s node level=%d", sDir[uDir], node->_uBitCount);
	  node = node->_child[uDir];
	  //node->uBitCount = node->parent->uBitCount << 1 | uDir;
	  node->_uBitCount = node->_parent->_uBitCount + 1;
      }
      else if (ST_HFMS_IS_LEAF(node)){
	  stDebug("tree %s level=%x leaf ascii=%d", sDir[uDir], node->_uBitCount,
		  ST_HFMS_CHAR_VAL(handler, node));
	  handler->_uHeight = max(node->_uBitCount - 1, handler->_uHeight);
	  node = node->_parent;
      }
      else {
	  assert(0);
      }
  }
  stDebug("tree height=%d", handler->_uHeight);

  return 0;
}

