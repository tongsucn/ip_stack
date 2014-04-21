#include <stdlib.h>
#include <stdio.h>

#include "ip_route.h"

route_table_t *route_table_head = NULL;

int init_route_table()
{
	free(route_table_head);

	route_table_head = (route_table_t *)malloc(sizeof(route_table_t));

	route_table_head->dest_addr = 0;
	route_table_head->interface = 0x17171701;
	route_table_head->next = NULL;

	return 0;
}

void route_table_add(__u32 dest_addr, __u32 interface)
{
	route_table_t *iter = route_table_head;
	route_table_head = (route_table_t *)malloc(sizeof(route_table_t));

	route_table_head->dest_addr = dest_addr;
	route_table_head->interface = interface;
	route_table_head->next = iter;
}

int route_table_modify(__u32 dest_addr, __u32 interface)
{
	route_table_t *iter = route_table_head;

	while (iter != NULL)
	{
		if (dest_addr == iter->dest_addr)
			iter->interface = interface;
		iter = iter->next;
	}

	return iter ? -1 : 0;
}

int route_table_delete(__u32 dest_addr)
{
	if (!route_table_head)
		return -1;

	route_table_t *iter = route_table_head;

	if (route_table_head->dest_addr == dest_addr)
	{
		route_table_head = route_table_head->next;
		free(iter);
	}

	while (iter->next
		&& iter->next->dest_addr != dest_addr)
		iter = iter->next;

	if (iter->next)
	{
		route_table_t *tmp = iter->next;
		iter->next = tmp->next;
		free(tmp);

		return 1;
	}
	else if (iter->dest_addr == dest_addr)
	{
		route_table_t *tmp = route_table_head;
		while (tmp->next != iter && (tmp = tmp->next));

		tmp->next = NULL;
		free(tmp);

		return 1;
	}
	else
		return 0;
}

void route_table_clear()
{
	route_table_t *iter = route_table_head;
	while (route_table_head)
	{
		route_table_head = route_table_head->next;
		free(iter);
		iter = route_table_head;
	}
}
