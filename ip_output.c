#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>

#include "ip_output.h"
#include "ip_input.h"
#include "ip_route.h"

static __u8 uid_pool[65536];

extern route_table_t *route_table_head;

int ip_send(int length, __u8 protocol, __u32 dest_addr, void *data)
{
	ip_frag_list_t *frag_list, *head;

	if ((head = frag_list = ip_frag(length, 255, protocol, dest_addr, data)) == NULL)
		return -1;

	for (; frag_list != NULL; frag_list = frag_list->next)
		if (ip_send_checksum(frag_list->frag) < 0
			|| ip_forward(frag_list->frag) < 0)
		{
			free_frag_list(head);
			return -1;
		}

	return 0;
}

ip_frag_list_t *ip_frag(int length, __u8 ttl, __u8 protocol,
				__u32 dest_addr, void *data)
{
	if (length < 1 || ttl < 0 || !data)
		return NULL;

	ip_frag_list_t *head, *iter;
	head = iter = (ip_frag_list_t *)malloc(sizeof(ip_frag_list_t));
	head->next = NULL;

	char *byte_data = data;
	int i, node_num = length / 512;
	node_num += length % 512 && 1;

	for (i = 1; i < node_num; ++i)
	{
		iter->next = (ip_frag_list_t *)malloc(sizeof(ip_frag_list_t));
		iter = iter->next;
		iter->next = NULL;
	}

	__u16 id = get_id(), offset = 0;
	__u32 sour_addr = get_local_addr();
	int data_length;

	iter = head;

	for (; iter != NULL; iter = iter->next)
	{
		data_length = (length > 511 ? 512 : length);

		iter->frag = (ip_fragment_t *)malloc(sizeof(ip_fragment_t));
		iter->frag->head = (iphdr_t *)malloc(sizeof(iphdr_t));
		iter->frag->head->version = 0x4;
		iter->frag->head->ihl = 0x5;
		iter->frag->head->tos = 0x10;
		iter->frag->head->tot_len = 20 + data_length;
		iter->frag->head->id = id;
		iter->frag->head->frag_off = (iter->next ? offset + 0x2000 : offset);
		iter->frag->head->ttl = ttl;
		iter->frag->head->protocol = protocol;
		iter->frag->head->check = 0;
		iter->frag->head->saddr = sour_addr;
		iter->frag->head->daddr = dest_addr;

		iter->frag->ip_data = malloc(data_length);
		memcpy(iter->frag->ip_data, &byte_data[offset * 8], data_length);

		length -= 512;
		offset += 64;
	}

	return head;
}

__u16 get_id()
{
	int uid;

	srand(time(0));

	while (uid_pool[uid = rand() % 65536]);

	return uid;
}

int reset_id(__u16 uid)
{
	uid_pool[uid] = 0;
	return 0;
}

int ip_send_checksum(ip_fragment_t *frag)
{
	if (frag == NULL)
		return -1;

	frag->head->check = calc_checksum(frag);

	return 0;
}

void free_frag_list(ip_frag_list_t *frag_list)
{
	if (!frag_list)
		return;

	ip_frag_list_t *del_ptr = frag_list;

	while (frag_list != NULL)
	{
		free(frag_list->frag->head);
		free(frag_list->frag->ip_data);
		free(frag_list->frag);
		del_ptr = frag_list;
		frag_list = frag_list->next;
		free(del_ptr);
	}
}

int ip_forward(ip_fragment_t *frag)
{
	if (frag == NULL)
		return -1;

	__u8 *addr_check = (__u8 *)&frag->head->daddr;

	if (ip_dest_check(frag))
		ip_rcv(frag);

	if (addr_check[3] == 0
			|| addr_check[3] == 255
			|| addr_check[0] == 0
			|| addr_check[0] == 127
			|| addr_check[0] == 255)
		return 0;

	if (!route_table_head)
		return -1;

	route_table_t *iter = route_table_head;

	while (iter->next && iter->dest_addr != frag->head->daddr)
		iter = iter->next;

	if (iter != NULL && frag->head->ttl-- > 0)
		return ip_output(iter->interface, frag) < 0 ? -1 : 0;
	else
		return -1;
}

int ip_output(__u32 dest_addr, ip_fragment_t *frag)
{
	int queue_id, dest_pid;

	send_data_t to_sent = {frag->head->tot_len + sizeof(long), frag};

	printf("Please enter destination PID: ");
	scanf("%d", &dest_pid);

	if (dest_addr == 0x17171701)
		queue_id = msgget(PUBLIC_MSG_QUEUE_ROUTE_KEY, IPC_CREAT);
	else
		queue_id = msgget(PUBLIC_MSG_QUEUE_RCV_KEY, IPC_CREAT);

	printf("queue_id == %d\n", queue_id);

	if (msgsnd(queue_id, &to_sent, to_sent.length, IPC_NOWAIT) == -1)
	{
		printf("Error!\n");
		return -1;
	}

	kill(dest_pid, SIGUSR1);

	return 0;
}
