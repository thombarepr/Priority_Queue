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

	while (i < 20)
	{
		buf = calloc(20, 1);
		sprintf((char*)buf, "Buf with prio - %d", i);
		pq_add(pq, buf, i);
		i++;
	}

	i = 0;
	while (i < 20)
	{
		buf = pq_remove(pq);
		printf("%s\n", (char*)buf);
		i++;
	}

	return 0;
}
