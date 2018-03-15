#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "malloc.h"
#include "pthread.h"
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;

typedef void (*callback_t)(void);

typedef enum
{
	PQ_STATUS_FAIL = 0,
	PQ_STATUS_PASS,
	PQ_STATUS_INVALID,
	PQ_STATUS_EMPTY,
	PQ_STATUS_MAX,
} pq_status_t;
/****************************************************************************************/
/* Function: pq_init
*  To create paririty queue. Queue must be created before any operation on it.
*  Same code can by used to create multiple queues, uniquely identified by handle returned on pq_init.
*  In args: high water mark, high water mark callback function pointer, 
*           low water mark, high water mark callback function pointer
*  Returns: Priority queue handle else NULL. 
*/
void *pq_init(uint16_t, callback_t, uint16_t, callback_t);

/* Function: pq_add
*  To add data to particular paririty queue. Queue must be created before any operation on it.
*  In args: Priority queue handler returned by pq_init
*           pointer to buf holding data, priority(higher number means higher priority).
*  Returns: PQ_STATUS_INVALID for invalid queue handle, 
	    PQ_STATUS_PASS if data added to queue, otherewise PQ_STATUS_FAIL.
*/
pq_status_t pq_add(void *, void *, uint16_t);

/* Function: pq_remove
*  To remove highest priority data from particular paririty queue. 
*  Queue must be created before any operation on it.
*  In args: Priority queue handler returned by pq_init
*  Returns: 1. PQ_STATUS_INVALID for invalid queue handle, 
	    PQ_STATUS_PASS if queue is not empty, otherewise PQ_STATUS_EMPTY.
*           2. Pointer to data returned in 2 arg.
*/
pq_status_t pq_remove(void *, void**);

/* Function: pq_delete
*  Delete paririty queue identified by queue handle. 
*  Queue must be created before deleting it.
*  In args: Priority queue handler returned by pq_init
*  Returns: 1. PQ_STATUS_INVALID for invalid queue handle, 
	    otherwise PQ_STATUS_PASS 
*/
pq_status_t pq_delete(void *);
