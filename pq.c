#include "pq.h"

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

node_t *bst_remove(node_t *root)
{
	if (root)
	{
		node_t *node = bst_remove(node->right);
		if (NULL == node)
		{
		}
	}
	return root;
}

bool_t pq_add(void *p, void *data, uint16_t prio)
{
	pq_t *pq = (pq_t *)p;
	bool_t ret = FALSE;
	
	if ((NULL != pq) && (PQ_HEADER == pq->header))
	{
		pthread_mutex_lock(&pq->lock);
		node_t *node = (node_t *)calloc(sizeof(node_t), 1);
		if (NULL != node)
		{
			ret = TRUE;
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
		}
		pq->count++;
		pthread_mutex_unlock(&pq->lock);

		if (pq->count > pq->high_water_mark && pq->hcb)
			pq->hcb();	
	}
	return ret;
}

bool_t pq_remove(void *p, void **data)
{
	pq_t *pq = (pq_t *)p;
	bool_t ret = FALSE;
	
	if ((NULL != pq) && (PQ_HEADER == pq->header))
	{
		pthread_mutex_lock(&pq->lock);
		node_t *node = bst_remove(pq->root);
		if (node)
		{
			if (NULL == node->parent)
				pq->root = node->left;
			*data = node->data;
			free(node);
			node = NULL;
		}
		pq->count--;	
		pthread_mutex_unlock(&pq->lock);
		if (pq->count < pq->low_water_mark && pq->lcb)
			pq->lcb();	
	}
	return ret;
}
