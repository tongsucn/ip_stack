#ifndef IP_ROUTE_H
#define IP_ROUTE_H

#include "data_types.h"

typedef struct route_table
{
	__u32			dest_addr;
	__u32			interface;
	struct route_table	*next;
} route_table_t;

int init_route_table();

void route_table_add(__u32 dest_addr, __u32 interface);

int route_table_modify(__u32 dest_addr, __u32 interface);

int route_table_delete(__u32 dest_addr);

void route_table_clear();

#endif // IP_ROUTE_H
