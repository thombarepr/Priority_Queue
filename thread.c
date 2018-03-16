#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "pq.h"
#define PQ_HIGH_WATER_MARK	(20)
#define PQ_LOW_WATER_MARK	(3)
#define MAX_OPS		(300)
#define MAX_PRIO	(500)
#define MIN_OPS		((MAX_OPS * 3) / 4)

#define DBG 1
#if DBG
#define THREAD_DBG(a, b...) \
do{ \
	printf(a, ##b); \
} while(0);
#else
#define THREAD_DBG(a, b...) 
#endif

typedef struct
{ 	
	pthread_t t;
	void *pq;
	uint8_t id;
	uint8_t ops;
} data_t;

typedef enum 
{ 
	PQ_OP_ADD = 0,
	PQ_OP_REMOVE,
	PQ_OP_DELETE,
	PQ_OP_MAX
} pq_op_e;

static uint8_t high_throttle;
static uint8_t low_throttle;

void high_watermark_cb(uint8_t throttle)
{
	high_throttle = throttle;
	THREAD_DBG("High water mark throttle %s.\n", throttle ? "ON":"OFF");
}

void low_watermark_cb(uint8_t throttle)
{
	low_throttle = throttle;
	THREAD_DBG("Low water mark throttle %s.\n", throttle ? "ON":"OFF");
}

void *thread(void *arg)
{
	data_t *d = (data_t *)arg;
	uint16_t prio = 0;
	/* Number of random operations to be performed */
	int num_ops = random() % MAX_OPS;
	char *buf = NULL;
	pq_status_t status = 0;

	THREAD_DBG("Thread - %d created.\n", d->id);
	while (num_ops--)
	{
		/* Selecting operation to perform randomly, this is to test multi thread support of priority queue.*/
		pq_op_e op = random() % PQ_OP_MAX;
		switch (op)
		{
			case PQ_OP_ADD:
			/* If high throttle ON don't add to queue */
			if (0 == high_throttle)
			{
				prio = (uint16_t)(random() % MAX_PRIO);
				buf = calloc(64, 1);
				if (buf)
				{
					sprintf(buf, "AddedBy: %d Prio: %hu", d->id, prio);
					if (PQ_STATUS_PASS != (status = pq_add(d->pq, (void*)buf, prio)))
					{
						/* If pq_add fails, free allocated buffer to avoid memory leak */
						free(buf);
						buf = NULL;
						if (PQ_STATUS_INVALID == status)
						{
							/* Queue deleted ?? */
							num_ops = 0;
							continue;
						}
					}
					else
					{
						THREAD_DBG("Data added: %s.\n", buf);
						d->ops++;
					}
				}
			}
			else
			{
				//THREAD_DBG("Thread %d: High water mark throttling on, ADD aborted.\n", d->id);
			}
			break;
	
			case PQ_OP_REMOVE:
			/* If low throttle ON don't remove from queue */
			if (0 == low_throttle)
			{
				if (PQ_STATUS_PASS == (status = pq_remove(d->pq, (void **)&buf)))
				{
					d->ops++;
					THREAD_DBG("Data removed by thread: %d  - %s.\n", d->id, buf);
					/* Free allocated buffer */
					if (buf)
					{
						free(buf);
						buf = NULL;
					}
				}
				else if (PQ_STATUS_INVALID == status)
				{
					/* Queue deleted ?? */
					num_ops = 0;
					continue;
				}
			}
			else
			{
				//THREAD_DBG("Thread %d: Low water mark throttling on, REMOVE aborted.\n", d->id);
			}
			break;
	
			case PQ_OP_DELETE:
			if (MIN_OPS < d->ops)
			{
				if (PQ_STATUS_PASS == pq_delete(d->pq))
				{
					THREAD_DBG("Priority queue deleted by thread %d.\n", d->id);
					/* If pq_delete op is selected randomly. 
	 			 	* Delete the queue, send cancel signal to other thread and exit 
 				 	*/ 			 	   	
					pthread_cancel(d->t);	
					pthread_exit(NULL);
				}
			}
			else
			{
				num_ops++;
			}
			break;

			default:
			THREAD_DBG("Invalid operation - %d !!!\n", op);
			break;
		}
		//sleep(1);
	}

	/* If all ops are completed cancel other thread and exit */
	pthread_cancel(d->t);
	pthread_exit(NULL);
	return NULL;
}

int main()
{
	data_t d1, d2;
	void *pq = pq_init(PQ_HIGH_WATER_MARK, high_watermark_cb, PQ_LOW_WATER_MARK, low_watermark_cb);
	assert(pq);

	d1.pq = pq;
	d1.id = 1;
	d1.ops = 0;
	d2.pq = pq;
	d2.id = 2;
	d2.ops = 0;

	/* Every thread use pthread_t from other thread's struct so that cancel request can be sent to other thread.*/	
	if (0 == pthread_create(&d2.t, NULL, thread, (void*)&d1))
	{
		if (0 == pthread_create(&d1.t, NULL, thread, (void*)&d2))
		{
			pthread_join(d1.t, NULL);
			pthread_join(d2.t, NULL);
		}
		else
		{
			pthread_cancel(d2.t);
			pthread_join(d2.t, NULL);
		}
	}
	/* In case none of the thread deleted queue.
 	 * If already deleted this function gracefully returns INVALID status 
 	 */
	pq_delete(pq);
	return 0;
}
