#ifndef IP_INPUT_H
#define IP_INPUT_H

#include "data_types.h"
#include "ip.h"

typedef struct defrag_queue
{
	int			queue_id;
	ip_fragment_t		*frag;
	struct defrag_queue	*x_next;
	struct defrag_queue	*y_next;
} defrag_queue_t;

static defrag_queue_t	*defrag_queue_table = NULL;

int ip_rcv(void *data);

int iphdr_check(const void *data);

int ip_dest_check(const ip_fragment_t *frag);

int ip_defrag(ip_fragment_t *frag);

int verify_queue(const defrag_queue_t *queue);

#endif // IP_INPUT_H
