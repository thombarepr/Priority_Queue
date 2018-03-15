#include "pq.h"
#define DEBUG 0
#if DEBUG
#define PQ_DEBUG(a,b...) printf(a, ##b);
#else
#define PQ_DEBUG(x) 
#endif
/****************************************************************************************/
/* Each data (void *data) is wrapped inside node struct */
typedef struct node{
	uint16_t prio;
	void *data;
	struct node *right;
	struct node *left;
	struct node *parent;
} node_t;

/* Priority tree struct kept here to avoid 'direct' access to priority queue fields 
* Access is allowed only using defined API's in pq.h 
*/
typedef struct {
	uint16_t header;
	uint16_t count;
	uint16_t high_water_mark;
	uint16_t low_water_mark;
	callback_t hcb;
	callback_t lcb;
	pthread_mutex_t lock;
	node_t *root;	
} pq_t;

#define PQ_HEADER	(uint16_t)(0x5A6B)
/****************************************************************************************/
/* Function: pq_init
*  To create paririty queue. Queue must be created before any operation on it.
*  Same code can by used to create multiple queues, uniquely identified by handle returned on pq_init.
*  In args: high water mark, high water mark callback function pointer, 
*           low water mark, high water mark callback function pointer
*  Returns: Priority queue handle 
*/
void *pq_init(uint16_t high_water_mark, callback_t hcb, uint16_t low_water_mark, callback_t lcb)
{
	pq_t *pq = (pq_t *)calloc(sizeof(pq_t), 1);

	if (NULL != pq)
	{
		pq->header = PQ_HEADER;
		pq->high_water_mark = high_water_mark;
		pq->hcb = hcb;
		pq->low_water_mark = low_water_mark;
		pq->lcb = lcb;
		if ((0 != pthread_mutex_init(&pq->lock, NULL)) || (high_water_mark < low_water_mark))
		{
			free(pq);
			pq = NULL;
		}
	}
	return pq;	
}

void bst_add(node_t *root, node_t *node)
{
	if (node->prio >= root->prio)
	{
		if (NULL == root->right)
		{
			root->right = node;
			node->parent = root;
		}
		else	
		{
			bst_add(root->right, node);
		}
	}
	else
	{
		if (NULL == root->left)
		{
			root->left = node;
			node->parent = root;
		}
		else	
		{
			bst_add(root->left, node);
		}
	}
}

node_t *bst_remove(pq_t *pq)
{
	node_t *node = pq->root;

	if (NULL != node)
	{
		while (NULL != node->right)
			node = node->right;

		if (NULL != node->parent)
		{
			node->parent->right = node->left;
			if (NULL != node->left)
				node->left->parent = node->parent;
		}
		else
		{
			pq->root = node->left;
			if (NULL != node->left)
				node->left->parent = NULL;
		}
	}
	return node;
}

/* Function: pq_add
*  To add data to particular paririty queue. Queue must be created before any operation on it.
*  In args: Priority queue handler returned by pq_init
*           pointer to buf holding data, priority(higher number means higher priority).
*  Returns: PQ_STATUS_INVALID for invalid queue handle, 
	    PQ_STATUS_PASS if data added to queue, otherewise PQ_STATUS_FAIL.
*/
pq_status_t pq_add(void *p, void *data, uint16_t prio)
{
	pq_t *pq = (pq_t *)p;
	pq_status_t ret = PQ_STATUS_INVALID;
	
	if ((NULL != pq) && (PQ_HEADER == pq->header))
	{
		pthread_mutex_lock(&pq->lock);
		node_t *node = (node_t *)calloc(sizeof(node_t), 1);
		if (NULL != node)
		{
			ret = PQ_STATUS_PASS;
			node->prio = prio;
			node->data = data;
			if (NULL == pq->root)
			{
				pq->root = node;
			}
			else
			{
				bst_add(pq->root, node);
			}
			pq->count++;
			ret = PQ_STATUS_PASS;
		}
		else
		{
			ret = PQ_STATUS_FAIL;
		}
		pthread_mutex_unlock(&pq->lock);

		if (pq->count > pq->high_water_mark && pq->hcb)
			pq->hcb();	
	}
	else
	{
		PQ_DEBUG("Invalid priority queue !!!\n");
	}
	return ret;
}

/* Function: pq_remove
*  To remove highest priority data from particular paririty queue. 
*  Queue must be created before any operation on it.
*  In args: Priority queue handler returned by pq_init
*  Returns: 1. PQ_STATUS_INVALID for invalid queue handle, 
	    PQ_STATUS_PASS if queue is not empty, otherewise PQ_STATUS_EMPTY.
*           2. Pointer to data returned in i/p arg 2.
*/
pq_status_t pq_remove(void *p, void **data)
{
	pq_t *pq = (pq_t *)p;
	pq_status_t ret = PQ_STATUS_INVALID;
	
	if ((NULL != pq) && (PQ_HEADER == pq->header))
	{
		pthread_mutex_lock(&pq->lock);
		node_t *node = bst_remove(pq);
		if (NULL != node)
		{
			if (NULL != data)
			{
				*data = node->data;
				ret = PQ_STATUS_PASS;
			}
			else
			{
				ret = PQ_STATUS_FAIL;
			}
			free(node);
			node = NULL;
			pq->count--;	
		}
		else
		{
			PQ_DEBUG("Priority queue is empty.\n");
			ret = PQ_STATUS_EMPTY;
		}
		pthread_mutex_unlock(&pq->lock);
		if (pq->count < pq->low_water_mark && pq->lcb)
			pq->lcb();	
	}
	else
	{
		PQ_DEBUG("Invalid priority queue !!!\n");
	}
	return ret;
}

/* Function: pq_delete
*  Delete paririty queue identified by queue handle. 
*  Queue must be created before deleting it.
*  In args: Priority queue handler returned by pq_init
*  Returns: 1. PQ_STATUS_INVALID for invalid queue handle, 
	    otherwise PQ_STATUS_PASS 
*/
pq_status_t pq_delete(void *p)
{
	pq_t *pq = (pq_t *)p;
	pq_status_t ret = PQ_STATUS_INVALID;

	if ((NULL != pq) && (PQ_HEADER == pq->header))
	{
		void *buf = NULL;
		while (1)
		{	
			buf = NULL;
			if (PQ_STATUS_EMPTY != pq_remove(pq, &buf))
			{
				if (NULL != buf)
				{
					free(buf);
					buf = NULL;
				}
			}
			else	
			{
				break;
			}
		}
		pq->header = 0;
		free(pq);
		pq = NULL;
		ret = PQ_STATUS_PASS;
	}
	else
	{
		PQ_DEBUG("Invalid priority queue !!!\n");
	}
	return ret;
}
