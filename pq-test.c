#include "pq.h"
void high_watermark_cb()
{
	printf("High water mark hit\n");
}

void low_watermark_cb()
{
	printf("Low water mark hit\n");
}

int main()
{
	void *pq = pq_init(15, high_watermark_cb, 3, low_watermark_cb);
	assert(pq);
	void *buf;
	uint8_t i = 0;
	uint16_t prio = 0;

	while (i < 20)
	{
		buf = calloc(20, 1);
		prio = (uint16_t)random();
		sprintf((char*)buf, "Buf with prio - %hu", (uint16_t)prio);
		pq_add(pq, buf, prio);
		i++;
	}

	i = 0;
	while (i < 20)
	{
		pq_remove(pq, &buf);
		printf("%s\n", (char*)buf);
		i++;
	}

	printf("%d\n", pq_remove(pq, &buf));
	printf("%d\n", pq_delete(pq));

	printf("%d\n", pq_add(pq, buf, prio));
	printf("%d\n", pq_remove(pq, &buf));
	printf("%d\n", pq_delete(pq));

	return 0;
}
