#include "stdio.h"
#include "assert.h"
#include "malloc.h"
#include "pthread.h"
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

typedef struct node{
	uint16_t prio;
	void *data;
	struct node *right;
	struct node *left;
	struct node *parent;
} node_t;

typedef void (*callback_t)(void);

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

typedef enum
{
	FALSE = 0,
	TRUE = 1,
} bool_t;

#define PQ_HEADER	(uint16_t)(0x5A6B)
void* pq_init(uint16_t high_water_mark, callback_t hcb, uint16_t low_water_mark, callback_t lcb);
bool_t pq_add(void *p, void *data, uint16_t prio);
void *pq_remove(void *p);
