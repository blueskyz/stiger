/**
 * @file   st_darray.h
 * @author Clark <zhangshizhuo@gmail.com>
 * @date   Tue May 15 10:26:07 2012
 * 
 * @brief  static huffman
 * 
 * 
 */

#include <alg/st_huffman_s.h>
#include <alg/st_heap.h>

typedef enum { ST_HFMS_TREE_NULL, ST_HFMS_TREE_NODE, ST_HFMS_TREE_LEAF } st_hfms_tree_type;

#define CODE_SIZE 0x01 << 8

typedef struct st_hfms_node st_hfms_node;
struct st_hfms_node{
  st_hfms_tree_type type;
  st_hfms_node* parent;
  uint32_t count; // count of all child
  uint32_t uBitCount; // level == bit length
  st_hfms_node* child[2]; // 0: left, 1: right
};

typedef struct {
  st_hfms_tree_type type;
  st_hfms_node* parent;
  uint32_t count; // weight 
  uint32_t uBitCount; // level == bit length
  BYTE* bitCode;
} st_hfms_leaf;

struct st_hfms{
  st_hfms_node* tree;
  uint16_t uHeight;
  uint32_t uLen;
  BYTE* data;
  st_hfms_leaf leaf[CODE_SIZE];
  st_hfms_node node[CODE_SIZE];
};

#define ST_HFMS_IS_NODE(pNode) (((st_hfms_node*)pNode)->type == ST_HFMS_TREE_NODE)
#define ST_HFMS_IS_LEAF(pLeaf) (((st_hfms_leaf*)pLeaf)->type == ST_HFMS_TREE_LEAF)


//------------------------------------------------------
// private
static int stHfmsByteUnitCmp(void* one, void* two)
{
  st_hfms_node* p1 = (st_hfms_node*)one;
  st_hfms_node* p2 = (st_hfms_node*)two;
  if (p1->count < p2->count){
	return 1;
  }
  else {
#ifdef DEBUG_DETAIL
	if (p1->type == ST_HFMS_TREE_LEAF){
	  stDebug("need exchange leaf p1=%u count=%u",
		  p1->byte, p1->count);
	}
	else {
	  stDebug("need exchange node p1 count=%u", p1->count);
	}
	if (p2->type == ST_HFMS_TREE_LEAF){
	  stDebug("need exchange leaf p2=%u count=%u end", 
		  p2->byte, p2->count);
	}
	else {
	  stDebug("need exchange node p2 count=%u", p2->count);
	}
#endif
	return -1;
  }
}

static int stHfmSCreateTree(st_hfms* handler)
{
  // statistic
  BYTE* input = handler->data;
  for (uint32_t i = 0 ; i < handler->uLen ; ++i){
	st_hfms_leaf* leaf = &handler->leaf[input[i]];
	if (leaf->count == 0){
	  leaf->type = ST_HFMS_TREE_LEAF;
	  leaf->parent = NULL;
	  leaf->bitCode = NULL;
	  leaf->uBitCount = 0;
	}
	leaf->count++;
  }

#define ST_HFMS_HEAP_LEN 256
  st_heap* heapHandler = stHeapNew(ST_HFMS_HEAP_LEN, stHfmsByteUnitCmp);
  if (NULL == heapHandler){
	stErr("new heap fail.");
	goto fail;
  }
  for (uint32_t i = 0 ; i < ST_HFMS_HEAP_LEN ; ++i){
	/*
	stDebug("i=%u, push ascii=%u, count=%u", 
		i, handler->unit[i].byte, handler->unit[i].count);
		*/
	if (0 == handler->leaf[i].count){
	  continue;
	}
	stHeapPush(heapHandler, &handler->leaf[i]);
  }

#ifdef DEBUG
  // output content
  stDebug("output byte content");
  for (uint32_t i = 0 ; i < ST_HFMS_HEAP_LEN ; ++i){
	if (0 == handler->leaf[i].count) continue;
	if (i < 128){
	  stDebug("ascii=%u, char=%c, times=%u", i, i, handler->leaf[i].count);
	}
	else {
	  stDebug("ascii=%u, times=%u", i, handler->leaf[i].count);
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
	// debug log
	if (pUnit[u2]->type == ST_HFMS_TREE_LEAF){
	  stDebug("0 ascii=%u, count=%u", 
		  (uint32_t)(pUnit[u2] - handler->leaf),
		  pUnit[u2]->count);
	}
	// debug end

	if(0 == u2){
	  pUnit[++u2] = stHeapPop(heapHandler);
	}
	if (NULL == pUnit[u2]){ // u2 == 1
	  // end
	  stDebug("find tree root");
	  handler->tree = (st_hfms_node*)pUnit[0];
	  handler->tree->uBitCount = 0;
	  break;
	}
	else{
	  // process and push back
	  st_hfms_node* pNode = &handler->node[uNodeIdx];
	  pUnit[0]->parent = pNode;
	  pUnit[1]->parent = pNode;
	  pNode->type = ST_HFMS_TREE_NODE;
	  pNode->parent = NULL;
	  pNode->count = pUnit[0]->count + pUnit[1]->count;
	  pNode->child[0] = (st_hfms_node*)pUnit[0];
	  pNode->child[1] = (st_hfms_node*)pUnit[1];
	  stHeapPush(heapHandler, pNode);

	  // debug log
	  if (pUnit[u2]->type == ST_HFMS_TREE_LEAF){
		stDebug("1 ascii=%u, count=%u", 
			(uint32_t)(pUnit[u2] - handler->leaf),
			pUnit[u2]->count);
	  }
	  stDebug("create node uNodeIdx=%u, count=%u", uNodeIdx, pNode->count);
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
  return 0;
}

static int stHfmSCompress(st_hfms* handler)
{
  assert(NULL != handler);
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
  handler->tree = NULL;
  return handler;
}

int stHfmSBuild(st_hfms* handler, BYTE* input, uint32_t uLen)
{
  // check argument
  handler->data = input;
  if (NULL == input){
	stErr("input data is NULL.");
	goto fail;
  }
  handler->uLen = uLen;
  if (0 == uLen){
	stErr("input data length is %u", uLen);
	goto fail;
  }

  // create huffman tree
  stHfmSCreateTree(handler);
  // bit encode
  stHfmSCalcBitCode(handler);
  // debug out tree
  stHfmSDebug(handler);
  // compress
  stHfmSCompress(handler);
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
  st_hfms_node* node = handler->tree;
  stDebug("print tree node root=%p", node);
  while(NULL != node){
	if (node->type == ST_HFMS_TREE_NODE){
		if (0 == node->child[uLeft]->uBitCount){
			uDir = uLeft;
		}
		else if (0 == node->child[uRight]->uBitCount){
			uDir = uRight;
		}
		else {
			node = node->parent;
			continue;
		}
		node = node->child[uDir];
		node->uBitCount = node->parent->uBitCount + 1;
		if (node->type == ST_HFMS_TREE_LEAF){
			stDebug("tree %s leaf=%p", sDir[uDir], node);
			node = node->parent;
		}
		else {
			stDebug("tree %s node=%p", sDir[uDir], node);
		}
	}
  }

  return 0;
}

