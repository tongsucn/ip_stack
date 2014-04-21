#include <stdlib.h>

#include "ip_input.h"

static int queue_id_count = 1;

int ip_rcv(void *data)
{
	ip_fragment_t *frag = data;

	if (iphdr_check(data) == -1)
		return -1;
	else if (ip_dest_check(frag))
		return ip_defrag(frag);
	else
		return ip_forward(frag);
}

int iphdr_check(const void *data)
{
	ip_fragment_t *frag = (ip_fragment_t *)data;
	__u8 head_length;

	if ((head_length = frag->head->ihl) < 5)
		return -1;

	return calc_checksum(frag) ? 0 : -1;
}

int ip_dest_check(const ip_fragment_t *frag)
{
	return (frag->head->daddr == get_local_addr())
		|| (frag->head->daddr == 0x7f000001) ? 1 : 0;
}

int ip_defrag(ip_fragment_t *frag)
{
	if (frag == NULL)
		return -1;

	defrag_queue_t *iter = defrag_queue_table;

	if (iter == NULL)
	{
		iter = defrag_queue_table
			= (defrag_queue_t *)malloc(sizeof(defrag_queue_t));

		iter->queue_id = queue_id_count++;
		iter->frag = frag;
		iter->x_next = NULL;
		iter->y_next = NULL;
	}
	else
	{
		while (iter != NULL
			&& !(iter->frag->head->id == frag->head->id
				&& iter->frag->head->protocol == frag->head->protocol
				&& iter->frag->head->saddr == frag->head->saddr
				&& iter->frag->head->daddr == frag->head->daddr))
			iter = iter->y_next;

		if (iter)
		{
			defrag_queue_t *queue_head = iter;

			if (frag->head->frag_off & 0x2000)
			{
				if ((frag->head->frag_off & 0x1fff) < ((iter->frag->head->frag_off & 0x1fff)))
				{
					queue_head = defrag_queue_table;
					while (queue_head != iter && queue_head->y_next != iter)
						queue_head = queue_head->y_next;

					queue_head->y_next = (defrag_queue_t *)malloc(sizeof(defrag_queue_t));
					queue_head = iter->y_next;
					queue_head->queue_id = iter->queue_id;
					queue_head->frag = frag;
					queue_head->x_next = iter;
					queue_head->y_next = iter->y_next;

					while (iter != NULL)
					{
						iter->y_next = queue_head;
						iter = iter->x_next;
					}
				}
				else
				{
					while (iter->x_next != NULL
						&& (iter->frag->head->frag_off & 0x1fff)
							< (iter->x_next->frag->head->frag_off & 0x1fff))
						iter = iter->x_next;

					if (iter->x_next)
					{
						defrag_queue_t *tmp = iter->x_next;

						iter->x_next = (defrag_queue_t *)malloc(sizeof(defrag_queue_t));
						iter = iter->x_next;
						iter->queue_id = tmp->queue_id;
						iter->frag = frag;
						iter->x_next = tmp;
						iter->y_next = queue_head;
					}
					else
					{
						iter->x_next = (defrag_queue_t *)malloc(sizeof(defrag_queue_t));
						iter = iter->x_next;
						iter->queue_id = queue_head->queue_id;
						iter->frag = frag;
						iter->x_next = NULL;
						iter->y_next = queue_head;
					}
				}
			}
			else
			{
				while (iter->x_next != NULL)
					iter = iter->x_next;

				iter->x_next = (defrag_queue_t *)malloc(sizeof(defrag_queue_t));
				iter = iter->x_next;
				iter->queue_id = queue_head->queue_id;
				iter->frag = frag;
				iter->x_next = NULL;
				iter->y_next = queue_head;
			}

			iter = queue_head;
		}
		else
		{
			iter = defrag_queue_table;
			while (iter->y_next != NULL)
				iter = iter->y_next;

			iter->y_next = (defrag_queue_t *)malloc(sizeof(defrag_queue_t));
			iter = iter->y_next;
			iter->queue_id = queue_id_count++;
			iter->frag = frag;
			iter->x_next = NULL;
			iter->y_next = NULL;
		}
	}

	return verify_queue(iter) ? iter->queue_id : 0;
}

int verify_queue(const defrag_queue_t *iter)
{
	defrag_queue_t *queue_head = (defrag_queue_t *)iter;

	if (iter->frag->head->frag_off & 0x1fff)
		return 0;

	while (iter->x_next != NULL)
		iter = iter->x_next;

	if (iter->frag->head->frag_off & 0x2000)
		return 0;

	__u16 curr_off = 0;
	iter = queue_head;

	while (iter->x_next != NULL)
	{
		curr_off += (iter->frag->head->tot_len
				- iter->frag->head->ihl * 2);

		if ((iter->x_next->frag->head->frag_off & 0x1fff) * 8
			!= curr_off)
			return 0;
	}

	return queue_head->queue_id;
}
